
#ifndef SINGLY_LINKED_LIST_GUARD
#define SINGLY_LINKED_LIST_GUARD 1

#include "std_lib_facilities.h"

template<typename Elem>
struct sLink {			// no prev pointer, unlike Link<> for List<Elem> above
  sLink* succ;
  Elem val;
  
  sLink(const Elem& elem) : val{elem}, succ{nullptr} {}
  // copy constructor of type Elem is assumed to exist
  sLink() : val{}, succ{nullptr} {}
};

template<typename Elem>
class sList{
public:
  using size_type = unsigned long;
  using value_type = Elem;
  // Since I define iterator and const_iterator newly in this template, no need to make aliases
  // of iterator and const_iterator
  
  sLink<Elem>* first;		// points to the 1st element
  sLink<Elem>* last;		// points to the last element (one before end())
  // Like the original List<> class above, I prepared both first and last for sList<> as well

  sList()
    : first{0}, last{first}, sz{0}
  {}
  
  sList(initializer_list<Elem> lst) : first{0}, last{first}, sz{0}
  {
    for(const auto e : lst)
      push_back(e);
  }
  
  // copy constructor
  sList(const sList<Elem>& lst) : first{0}, last{first}, sz{0}
  {
    // create the same number of sLink<Elem> objects, and copy lst's elements
    for(const auto a : lst)	// a is Elem type, not sLink<Elem> type
      push_back(a);
  }
  // copy assignment operator
  sList<Elem>& operator=(const sList<Elem>& a){
    this->~sList();		// reset
    for(const auto e : a)
      push_back(e);
        
    return *this;
  }
  
  // move constructor
  sList(const sList<Elem>&& lst) : first{0}, last{first}, sz{0}
  {
    // rob lst of its elements
    first = lst.first;
    last = lst.last;
    sz = lst.sz;

    // to prevent lst's elements from being destroyed in lst's destructor, set lst's sz to 0
    lst.sz = 0;
  }
  // move assignment operator
  sList<Elem>& operator=(const sList<Elem>&& a){
    // first, delete this sList's existing elements
    this->~sList();
    
    // then, the rest is the same as move constructor. Rob a of its resources
    first = a.first;
    last = a.last;
    sz = a.sz;
    // to prevent a's elements from being destroyed in a's destructor, set a's sz to 0
    a.sz = 0;

    return *this;
  }
  
  ~sList(){
    iterator p{begin()}, p2{iterator(this, p.curr->succ)};
    for(int i=0; i<sz; ++i){			     // delete except end() element
      delete p.curr;
      p = p2;
      ++p2;
      // if I didn't have p2, and only had p, since delete p.curr; deleted sLink<> pointed to
      // by p already, ++p would not succeed.
    }
    // to use this destructor in move and copy assignment operators, reset first, last and sz
    first = 0;			// 0 means pointing to end() element
    last = 0;
    sz = 0;
  }
  
  class iterator;
  iterator begin(){return iterator(this, first);}
  iterator end(){return iterator(this, 0);}

  class const_iterator;
  const_iterator begin()const {return const_iterator(this, first);}
  const_iterator end()const {return const_iterator(this, 0);}

  size_type size(){return sz;}
  
  iterator insert(iterator p, const Elem& v); // insert v into list before p
  iterator erase(iterator p);
  // I will not prepare a version of const_iterator, because using const_iterator is supposed
  // not to change the list

  iterator find_previous(iterator p);
  // This member function is new in sList. By traversing from first element, find the previous
  // element to p
  
  void push_back(const Elem& v); // insert v at end
  void push_front(const Elem& v); // insert v at front
  
  void pop_front();		  // remove the 1st element
  void pop_back();		  // remove the last element
  
  Elem& front(){
    if(begin()==end()) throw("Error in list<Elem>::front(). No element exists in this list.");
    return first->val;}		// the 1st element
  Elem& back(){
    if(begin()==end()) throw("Error in list<Elem>::front(). No element exists in this list.");
    return last->val;}			// the last element

  // Since these const front() and back() cannot change sList's internal state, its return type
  // becomes just a temporary copy of Elem, instead of Elem&
  Elem front() const {
    if(begin()==end()) throw("Error in list<Elem>::front(). No element exists in this list.");
    return first->val;}
  Elem back() const {
    if(begin()==end()) throw("Error in list<Elem>::front(). No element exists in this list.");
    return last->val;}
  
private:
  size_type sz;		// stores the number of elements (sLink<Elem>)
};

// Since a singly-linked list doesn't have a pointer to its previous sLink element, I change the
// meaning of sList<Elem>::insert(p,v). In List<Elem>::insert(p,v), the new element is inserted
// "before" the given iterator p. But here in sList<Elem>::insert(p,v), I insert the new sLink
// "after" the given iterator p.
template<typename Elem>
typename sList<Elem>::iterator sList<Elem>::insert(iterator p, const Elem& v){
    
  sLink<Elem>* new_l;

  if(p == begin() && begin() == end()){		// no element exists yet, and p points correctly
    // due to the corner case of p==end(), I assing new sLink<> inside each if clause

    // Since in this case, I cannot put the new element after any existing element (because
    // no element exists), only in this case, I put it at the front
    new_l = new sLink<Elem>(v);	
    first = new_l;		// change the front element
    last = new_l;
    new_l->succ = p.curr;	// == end()
  }
  // at this point, there is at least 1 element in this sList
  // different procedures depending on whether p is the last element or not
  else if(p == iterator(this, last)){
    // when p.curr == last, in addition to the normal procedures done in the other case, I
    // need to update last pointer
    new_l = new sLink<Elem>(v);
    new_l->succ = p.curr->succ;	// this is end(), but no problem
    p.curr->succ = new_l;
    // be careful in the order of update. If I update p.curr->succ before I pass it to
    // new_l->succ, that doesn't update them properly.

    last = new_l;		// update last, since this new_l is the new last element
  }
  else if(p != end()){		// the case of p.curr==first is included in this case
    // the following procedures are the same as the case above, except the update of last
    new_l = new sLink<Elem>(v);
    new_l->succ = p.curr->succ;	// this is end(), but no problem
    p.curr->succ = new_l;
  }
  else{
    throw runtime_error("Error in sList<Elemt>::insert(). The given iterator doesn't point to any of the elements, or points to end(). In this version of insert(p,v), a new element is inserted AFTER p");
  }
  ++sz;
  return iterator(this, new_l);
}

// Since singly-linked list doesn't have a back pointer, I made a new member to find the previous
// element to p at the cost of traversing from the first element (O(n), where n is the # of
// elements)
// When the previous element cannot be found, return end() (== iterator(this,0))
template<typename Elem>
typename sList<Elem>::iterator sList<Elem>::find_previous(iterator p){
  if(p.curr==first)
    throw runtime_error("Error in sList<Elem>::find_previous(iterator p). The iterator p is pointing to the first element.");
  else if(sz<2)
    throw runtime_error("Error in sList<Elem>::find_previous(iterator p). The number of elements in this sList is either 0 or 1, so no previous element exists to p.");
  else{
    // traverse from the 1st elem
    sLink<Elem>* t{first};
    for(int i=0; i<sz; ++i){
      if(t->succ == p.curr)
	return iterator(this, t);
      t = t->succ;
    }
  }
  // reaching here means traversing all elements in this sList cannot find p
  return end();
}

// Since there is no back pointer in sList, to update the p's previous element's succ pointer,
// I need to use find_previous()
// Since I felt erase() is one of the central operation to list, I didn't eliminate this.
template<typename Elem>
typename sList<Elem>::iterator sList<Elem>::erase(iterator p){
  iterator k{p}; ++k;			// points to the next element to p. Used as return value

  if(begin()==end())
    throw runtime_error("Error in sList<Elemt>::erase(). No element exists in this list.");

  if(p == begin() && sz==1){
    // update both first and last, since by this erase(), this sList has no element
    first = p.curr->succ;
    last = p.curr->succ;;
  }
  else if(p == begin() && sz>1){
    // only update first
    first = p.curr->succ;
  }
  else if(p.curr == last){
    iterator q{find_previous(p)}; // q points to the previous sLink<> to p
    q.curr->succ = p.curr->succ;
    // update last
    last = q.curr;
  }
  else if(p != end()){   // means p points to one of middle elements
    // first check if p really points to an element of this sList
    if(p.lst != this)
      throw runtime_error("Error in sList<Elemt>::erase(p). This iterator p points to an element of a different sList<Elem>.");

    iterator q{find_previous(p)}; // q points to the previous sLink<> to p
    q.curr->succ = p.curr->succ;
    // the difference from the case p.curr==last is whether last is updated or not
    // in this case, last stays the same.
  }
  else{				// when p==end()
    throw runtime_error("Error in sList<Elemt>::erase(). The given iterator points to end()");
  }
  
  delete p.curr;
  --sz;
  return k;
}

// In this function, although sLink doesn't have a back pointer, since we can use last,
// push_back() works
template<typename Elem>
void sList<Elem>::push_back(const Elem& v){
  if(begin()==end()){		// no element exists yet
    first = new sLink<Elem>(v);
    last = first;
    first->succ = 0;		// it seems ok to put 0 into a pointer
  }
  else{
    sLink<Elem>* p{new sLink<Elem>(v)};
    // first, connect the forward pointer from p to end()
    p->succ = 0;
    // then, connect the forwar pointer from last to p
    last->succ = p;
    // lastly, update last
    last = p;
  }
  ++sz;
}

template<typename Elem>
void sList<Elem>::push_front(const Elem& v){
  if(begin()==end()){		// no element exists yet
    first = new sLink<Elem>(v);
    last = first;
    first->succ = 0;		// iterator(this,0) == end()
  }
  else{
    sLink<Elem>* p{new sLink<Elem>(v)};
    // first, connect the 2 pointers from p to first
    p->succ = first;
    // then, update first
    first = p;
  }
  ++sz;
}

template<typename Elem>
void sList<Elem>::pop_front(){
  if(begin()==end())
    throw runtime_error("Error in list<Elemt>::pop_front(). No element exists in this list.");

  if(sz == 1){
    // in this case, both first and last are moved to pointing to the end() element
    delete first;
    first = 0;			// iterator(this,0) == end()
    last = 0;
  }
  else{				// sz > 1
    // in this case, only first pointer is moved to the successor
    sLink<Elem>* p{first};
    first = first->succ;
    delete p;
  }
  --sz;
}

// pop_back() can also be implemented with find_previous() member, but I decided not to do that,
// since pop_back() seems not as essential an operator as erase()

// Since in singly-linked list, there is no back pointers, I deleted the backward operator --
template<typename Elem>
class sList<Elem>::iterator {
public:
  sLink<Elem>* curr;		// current link
  const sList<Elem>* lst;
  
  iterator(const sList<Elem>* llst, sLink<Elem>* p) : curr{p}, lst{llst} {}

  iterator& operator++(){	// forward
    if(lst->end().curr == curr){				   // <= curr==0
      curr = lst->begin().curr;
      return *this;
    }
    curr = curr->succ; return *this;
  }
  Elem& operator*(){
    if(curr == 0)
      throw runtime_error("Error in Lst<Elem>::iterator's dereference operator *. You try to dereference end() element.");
    return curr->val;}			   // dereference (*iterator)
  sLink<Elem>* operator->(){return curr;}
  
  bool operator==(const iterator& b) const {return curr==b.curr;}
  // I don't have to compare lst with b.lst, because if curr==b.curr, both iterators point to
  // the same element, which must be in the same List<>
  bool operator!=(const iterator& b) const {return !(*this==b);}
};

// removed operator--() from List<Elem>::const_iterator, and the rest is the same
template<typename Elem>
class sList<Elem>::const_iterator {
public:
  sLink<Elem>* curr;		// current link
  const sList<Elem>* lst;	// pointer to the list
  
  const_iterator(const sList<Elem>* llst, sLink<Elem>* p) : curr{p}, lst{llst} {}

  const_iterator operator++(){	 // forward
    if(lst->end().curr == curr){				   // <= curr==0
      curr = lst->begin().curr;
      return *this;
    }
    curr = curr->succ; return *this;
  }
  Elem operator*()const{
    if(curr == 0) throw runtime_error("Error in Lst<Elem>::const_iterator's dereference operator *. You try to dereference end() element.");
    return curr->val;
  }			   // dereference (*iterator)
  const sLink<Elem>* operator->() const {return curr;}

  bool operator==(const const_iterator& b) const
  {return curr==b.curr && lst->first==b.lst->first;}
  bool operator!=(const const_iterator& b) const {return !(*this==b);}
};


#endif // SINGLY_LINKED_LIST_GUARD
