
#ifndef DOUBLY_LINKED_LIST_GUARD
#define DOUBLY_LINKED_LIST_GUARD 1

#include "std_lib_facilities.h"

template<typename Elem>
struct Link {
  Link* prev;
  Link* succ;
  Elem val;
  
  Link(const Elem& elem) : val{elem}, prev{nullptr}, succ{nullptr} {}
  // copy constructor of type Elem is assumed to exist
  Link() : val{}, prev{nullptr}, succ{nullptr} {}
};

// to distinguish this list class from std::list, I capitalize the first letter of the class name
// In ex 13, I modified List so that for end(), it points to just 0, instead of using an empty
// Link<Elem> object.
template<typename Elem>
class List{
public:
  using size_type = unsigned long;
  using value_type = Elem;
  // Since I define iterator and const_iterator newly in this template, no need to make aliases
  // of iterator and const_iterator
  
  Link<Elem>* first;		// points to the 1st element
  Link<Elem>* last;		// points to the last element (one before end())
  // last, and sz members can be omitted, at the cost of traversing the entire list when we
  // need these (when we need last, traverse before end(), and when we need sz, also traverse
  // all the elements and count the number). I think that is the meaning of the problem
  // description of ex 13 that "... that way, the size of an empty list can be equal to the
  // size of a single pointer".

  List()
    : first{0}, last{first}, sz{0}
      // both point to the Link<Elem> element which is the end() element, so this element is
      // not counted as 1 element.
      // It assumes that Elem type has default (empty) constructor
      // <- in ex 13, I changed it to pointing to 0
  {
    // first->prev = nullptr;
    // first->succ = nullptr;
  }

  // to be implemented: copy constructor, copy assignment operator, move constructor, move
  //                    assignment, initializer_list constructor

  
  List(initializer_list<Elem> lst) : first{0}, last{first}, sz{0}
  {
    for(const auto e : lst)
      push_back(e);
  }
  
  // copy constructor
  List(const List<Elem>& lst) : first{0}, last{first}, sz{0}
  {
    // create the same number of Link<Elem> objects, and copy lst's elements
    for(const auto a : lst)	// a is Elem type, not Link<Elem> type
      push_back(a);
  }
  // copy assignment operator
  List<Elem>& operator=(const List<Elem>& a){
    /*
    // use existing elements and copy a's elements into them
    iterator p{begin()};
    const_iterator q{a.begin()};
    for(int i=0; i<sz && i<a.sz; ++i, ++p, ++q){
      *p = *q;
    }
    // if a.sz == this->sz, the above for-loop is all we need
    
    // if a.sz > this->sz, we need push_back() the additional elements to this List<>
    // the following for-loop is not executed if q already reached a.end() in the 1st for-loop
    // above
    for(; q!=a.end(); ++q, ++p){	// increment p as well, for the next for-loop
      push_back(*q);
      cout << "*q= " << *q << endl;
    }
    cout << "### *p= " << *p << endl;
    // if a.sz < this->sz, we need to delete the extra elements of this List<>
    // if, in the previous for-loop above, p reached end(), this for-loop is not executed
    for(; p!=end();)
      p = erase(p);			// returned iterator is the next element of given p

    */ // the above code has some errors, which have not been fixed
    
    // this code would look much easier if I first delete all elements of this List by calling
    // this->~List(), and just push_back() a's elements.
    // The time complexity is O(n+m), where n is the size of this List, and m is the size of
    // a. This is the same as the above more complex code, whose complexity is O(n) if n>m,
    // and O(m) if m>n.
    this->~List();		// reset
    for(const auto e : a)
      push_back(e);
        
    return *this;
  }
  
  // move constructor
  List(const List<Elem>&& lst) : first{0}, last{first}, sz{0}
  {
    // rob lst of its elements
    first = lst.first;
    last = lst.last;
    sz = lst.sz;

    // to prevent lst's elements from being destroyed in lst's destructor, set lst's sz to 0
    lst.sz = 0;
  }
  // move assignment operator
  List<Elem>& operator=(const List<Elem>&& a){
    // first, delete this List's existing elements
    this->~List();
    sz = 0;
    
    // then, the rest is the same as move constructor. Rob a of its resources
    first = a.first;
    last = a.last;
    sz = a.sz;
    // to prevent a's elements from being destroyed in a's destructor, set a's sz to 0
    a.sz = 0;

    return *this;
  }
  
  ~List(){
    iterator p{begin()}, p2{iterator(this, p.curr->succ)}; // p2 points to the next Link<> to p
    for(int i=0; i<sz; ++i){			     // delete except end() element
      delete p.curr;
      p = p2;
      ++p2;
    }
    // in the end, delete end(). now p points to end() (p2 holds iterator(nullptr))
    //delete p.curr;
    // <- in ex 13, I replaced end()'s Link<Elem> with just 0

    // to use this destructor in move and copy assignment operators, reset first, last and sz
    first = 0;
    last = 0;
    sz = 0;
  }
  
  class iterator;
  iterator begin(){return iterator(this, first);}
  iterator end(){
    // if(sz) return iterator(last->succ);
    // return iterator(last);
    // if no element but the empty end element exists, return the pointer to it, so that users
    // can use a common practice of if(list.begin()==list.end()) to check if the container is
    // empty or not.

    //return iterator(e);
    // If I refer to "last" pointer to get the end() element, that makes the code of push_back()
    // and others confusing, when the updating of variable last is involved. So I use the special
    // pointer to the end() element.

    return iterator(this, 0);
    // in ex 13 I represent end() with just 0
  }

  class const_iterator;
  const_iterator begin()const {return const_iterator(this, first);}
  const_iterator end()const {
    // if(sz) return const_iterator(last->succ);
    // return const_iterator(last);

    //return const_iterator(e);

    return const_iterator(this, 0);
  }

  size_type size(){return sz;}
  
  iterator insert(iterator p, const Elem& v); // insert v into list before p
  iterator erase(iterator p);
  // I will not prepare a version of const_iterator, because using const_iterator is supposed
  // not to change the list

  
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

  Elem front() const {
    if(begin()==end()) throw("Error in list<Elem>::front(). No element exists in this list.");
    return first->val;}
  Elem back() const {
    if(begin()==end()) throw("Error in list<Elem>::front(). No element exists in this list.");
    return last->val;}
  
private:
  size_type sz;		// stores the number of elements (Link<Elem>)
  // using size_type = unsigned long; has to come before using such aliases, even within class
  // declarations.
  //Link<Elem>* e;
  // to simplify end() function, I prepare the pointer to end() element
  // <- in ex 13, I represent end() with just 0
};

template<typename Elem>
typename List<Elem>::iterator List<Elem>::insert(iterator p, const Elem& v){
  // <- without the keyword "typename", the compiler seems to feel hard to identify if
  //    List<Elem>::iterator is a name of some type or not. There are some lengthy explanations
  //    for this in the following sites, but I don't really understand it.
  // https://stackoverflow.com/questions/610245/
  // https://stackoverflow.com/questions/60277129/
  
  Link<Elem>* new_l;

  if(p == begin() && begin() == end()){		// no element exists yet, and p points correctly
    new_l = new Link<Elem>(v);	
    first = new_l;		// change the front element
    last = new_l;
    new_l->succ = p.curr;
    new_l->prev = nullptr;
    //p.curr->prev = new_l; // in this case, p is end()
    // <- in ex 13, I represent end() with just 0, so there is no end() element of Link<>
  }  
  // different procedures depending on whether p is the front element or not
  else if(p == begin()){
    // the difference of the case from the case above is the update of last
    
    // due to the corner case of p==end(), I assing new Link<> inside each if clause
    new_l = new Link<Elem>(v);	
    first = new_l;		// change the front element
    new_l->succ = p.curr;
    new_l->prev = nullptr;
    p.curr->prev = new_l;
  }
  else if(p != end()){		// the case of p.curr==last is included in this case
    new_l = new Link<Elem>(v);
    new_l->succ = p.curr;
    new_l->prev = p.curr->prev;
    p.curr->prev->succ = new_l;
    p.curr->prev = new_l;
    // be careful, the order of these operations matter. If p.curr->prev = new_l; comes before
    // p.curr->prev->succ = new_l;, p.curr->prev's value is changed, so the latter operation
    // doesn't work as expected.
  }
  else if(p == end()){		// when p is end(), this is the same as push_back()
    push_back(v);
  }
  else{
    throw runtime_error("Error in list<Elemt>::insert(). The given iterator doesn't point to any of the elements");
  }
  ++sz;
  return iterator(this, new_l);
}

template<typename Elem>
typename List<Elem>::iterator List<Elem>::erase(iterator p){
  iterator k{++p};			// points to the next element to p
  --p;					// return p to its original position
  if(begin()==end())
    throw runtime_error("Error in list<Elemt>::erase(). No element exists in this list.");

  if(p == begin() && sz == 1){
    // only 1 element exists. This case needs to be separated from the case of
    // p == begin() && sz > 1 in ex 13, because in this case, p.curr->succ is 0, instead of
    // end()'s Link<> element
    first = p.curr->succ;	// == 0 (end())
    last = p.curr->succ;
  }
  else if(p == begin() && sz > 1){
    first = p.curr->succ;
    p.curr->succ->prev = nullptr;
  }
  else if(p.curr == last){
    // last->prev->succ = last->succ;
    // last->succ->prev = last->prev;
    // see my comment in the case of p!=end() below. Order of Link pointer updating operations
    // matters.

    p.curr->prev->succ = p.curr->succ; // p.curr->succ is end() in this case
    //p.curr->succ->prev = p.curr->prev; // p.curr->succ is end()
    // <- in ex 13, I represent end() with just 0, so there is no end() element of Link<>

    // Since p's succ or prev are not updated, they can be used reliably here too
    last = p.curr->prev;
    // delete p.curr;
    // iterator p2{end()};
    // --p2;
    // last = p2.curr;
    // Notice: like the 4 lines above, if, to specify the new last element, I use end(), and
    //   try to call it delete p.curr;, since in this case, last == p.curr,
    //   and inside end(), last is used, that causes an error.
  }
  else if(p!=end()){		// means p points to one of middle elements
    // the same as the case of p.curr == last, except updating last
    
    // last->prev->succ = last->succ;
    // last->succ->prev = last->prev;
    // as written in the comment in insert(), inside the case of p!=end(), the order of these
    // pointer update operations matter. In this case, since whichever order I choose, the latter
    // update is affected by the previous one, I use p.curr to update its back and forth elements,
    // since p's succ and prev stay the same in these operations
    p.curr->prev->succ = p.curr->succ;
    p.curr->succ->prev = p.curr->prev;
  }
  else{
    // I think in this case-separation, even an iterator that doesn't point to any element
    // is classified into p!=end() case above
    throw runtime_error("Error in list<Elemt>::erase(). The given iterator doesn't point to any of the elements");
  }
  delete p.curr;
  --sz;
  return k;
}

template<typename Elem>
void List<Elem>::push_back(const Elem& v){
  if(begin()==end()){		// no element exists yet
    first = new Link<Elem>(v);
    last = first;
    first->prev = nullptr;
    // first->succ = e;
    // <- in ex 13, I represent end() with just 0, so there is no end() element of Link<>
    first->succ = 0;		// it seems ok to put 0 into a pointer
  }
  else{
    Link<Elem>* p{new Link<Elem>(v)};
    // first, connect the 2 pointers from p to last and end()
    //p->succ = e;
    // <- in ex 13, I represent end() with just 0, so there is no end() element of Link<>
    p->succ = 0;
    p->prev = last;
    // then, connect 2 pointers from last and end() to p
    //e->prev = p;
    // <- in ex 13, I represent end() with just 0, so there is no end() element of Link<>
    last->succ = p;
    // lastly, update last
    last = p;
  }
  ++sz;
  // Be careful: if I forget ++sz, --sz can happen when sz==0. Since sz is unsigned long,
  // meaning no negative value is allowed, --sz in sz==0 makes sz the maximum value of
  // unsigned long, which is 18446744073709551615
}

template<typename Elem>
void List<Elem>::push_front(const Elem& v){
  if(begin()==end()){		// no element exists yet
    first = new Link<Elem>(v);
    last = first;
    first->prev = nullptr;
    //first->succ = e;
    // <- in ex 13, I represent end() with just 0, so there is no end() element of Link<>
    first->succ = 0;		// 0 == end()
  }
  else{
    Link<Elem>* p{new Link<Elem>(v)};
    // first, connect the 2 pointers from p to first
    p->succ = first;
    p->prev = nullptr;		// p becomes first
    // then, connect the prev pointer of old (first has not been updated yet) first to p
    first->prev = p;
    // lastly, update first (if this update comes before first->prev = p; that would cause
    // errors)
    first = p;
  }
  ++sz;
}

template<typename Elem>
void List<Elem>::pop_front(){
  if(begin()==end())
    throw runtime_error("Error in list<Elemt>::pop_front(). No element exists in this list.");

  if(sz == 1){
    // in this case, both first and last are moved to pointing to the end() element
    delete first;
    // first = e;
    // last = e;
    // <- in ex 13, I represent end() with just 0, so there is no end() element of Link<>
    first = 0;
    last = 0;
  }
  else{				// sz > 1
    // in this case, only first pointer is moved to the successor
    Link<Elem>* p{first};
    first = first->succ;
    delete p;
  }
  --sz;
}

template<typename Elem>
void List<Elem>::pop_back(){
  if(begin()==end())
    throw runtime_error("Error in list<Elemt>::pop_back(). No element exists in this list.");

  if(sz == 1){
    // in this case, both first and last are moved to pointing to the end() element
    delete first;
    // first = e;
    // last = e;
    // <- in ex 13, I represent end() with just 0, so there is no end() element of Link<>
    first = 0;
    last = 0;
  }
  else{				// sz > 1
    // in this case, only last pointer is moved to the predecessor
    Link<Elem>* p{last};
    last = last->prev;
    //last->succ = e;		// connect the new last's successor to end()
    // <- in ex 13, I represent end() with just 0, so there is no end() element of Link<>
    last->succ = 0;
    delete p;
  }
  --sz;
}

// define class iterator declared inside list<Elem> (p727)
template<typename Elem>
class List<Elem>::iterator {
public:
  // made curr public, because I wanted to refer to it in list<Elem>::insert() from an iterator
  // object
  Link<Elem>* curr;		// current link
  //Link<Elem>* pre; 		// previous link. used to go back to previous link from end(),
  // since in ex 13, end() is represented by just 0, instead of Link<Elem>. So in ex 13,
  // curr->prev cannot be used for that purpose.

  // From this kind of a nested class (iterator is nested within List<Elem>), it cannot access
  // any member of the nesting class List<Elem> in C++. In C++, a nested class is not connected
  // to any instance of its nesting class.

  // To achive circulation iteration like STL list<> (this means ++end() points to the 1st
  // element of the list), and to go back to the last element by --end() (in ex 13, end() is
  // just 0 instead of Link<Elem>, so curr->prev cannot be used), I decided to take the
  // reference to the List<> instance.
  // <- reference cannot be changed later (it is basically a constant pointer, List<Elem> *const)
  //    , so declaring a reference as a member automatically delete assignment operator=(). 
  //    Since that's inconvenient, I made a reference to a pointer const List<Elem>*, which
  //    can be changed later.
  //    The difference between const pointer and pointer to const, see
  //    https://stackoverflow.com/questions/1143262/
  const List<Elem>* lst;
  // Since this is pointer to const, its pointer can be changed later. But it cannot change
  // the pointed object.
  // This additional 8 bits required for this additiona pointer lst is 1 cost of achieving
  // such a circulation iterator (the other cost is the relatively complex code compared to
  // the previous version of iterator).
  iterator(const List<Elem>* llst, Link<Elem>* p) : curr{p}, lst{llst} {}

  iterator& operator++(){	// forward
    if(lst->end().curr == curr){				   // <= curr==0
      curr = lst->begin().curr;
      return *this;
    }
    curr = curr->succ; return *this;
  }
  iterator& operator--(){	// backward
    if(lst->end().curr == curr){	// <= curr == 0
      curr = lst->last;		// go back to the last
      return *this;
    }
    else if(curr == lst->begin().curr){ // from begin(), go to the last element, not end()
      // this behavior is the same as that of STL's iterator
      curr = lst->last;
      return *this;
    }
    curr = curr->prev; return *this;
  }
  Elem& operator*(){
    if(curr == 0) throw runtime_error("Error in Lst<Elem>::iterator's dereference operator *. You try to dereference end() element.");
    return curr->val;}			   // dereference (*iterator)
  Link<Elem>* operator->(){return curr;}
  // This works because iterator-> is interpreted as (iterator.operator->())-> (notice another
  // arrow operator is added at the back). See my comment in operator-> in ex 10 of ch19
  
  bool operator==(const iterator& b) const {return curr==b.curr;}
  // I don't have to compare lst with b.lst, because if curr==b.curr, both iterators point to
  // the same element, which must be in the same List<>
  bool operator!=(const iterator& b) const {return !(*this==b);}
};

// define class const_iterator declared inside list<Elem>. This is almost just a copy of
// list<Elem>::iterator class, but in order for users of const_iterator not to place it
// as lvalue, I changed the return types of operator++, --, and * to just a temporary copy
// of the counterparts in class list<Elem>::iterator
template<typename Elem>
class List<Elem>::const_iterator {
public:
  Link<Elem>* curr;		// current link
  const List<Elem>* lst;	// pointer to the list
  
  const_iterator(const List<Elem>* llst, Link<Elem>* p) : curr{p}, lst{llst} {}

  const_iterator operator++(){	 // forward
    if(lst->end().curr == curr){				   // <= curr==0
      curr = lst->begin().curr;
      return *this;
    }
    curr = curr->succ; return *this;
  }
  // if the user tries to go past the element one past the last element, curr is nullptr? and
  // nullptr->succ; would cause segmentation fault.
  const_iterator operator--(){	 // backward
    if(lst->end().curr == curr){	// <= curr == 0
      curr = lst->last;		// go back to the last
      return *this;
    }
    else if(curr == lst->begin().curr){ // from begin(), go to the last element, not end()
      // this behavior is the same as that of STL's iterator
      curr = lst->last;
      return *this;
    }
    curr = curr->prev; return *this;
  }
  Elem operator*()const{
    if(curr == 0) throw runtime_error("Error in Lst<Elem>::const_iterator's dereference operator *. You try to dereference end() element.");
    return curr->val;
  }			   // dereference (*iterator)
  const Link<Elem>* operator->() const {return curr;}

  bool operator==(const const_iterator& b) const
  {return curr==b.curr;}
  bool operator!=(const const_iterator& b) const {return !(*this==b);}
};


#endif // DOUBLY_LINKED_LIST_GUARD
