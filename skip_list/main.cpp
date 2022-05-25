

#include "./std_lib_facilities.h"

template<typename T>
struct Skip_list {
  // T& key(){return k;}
  // Since the return type is a reference, k can be changed through this function, i.e.
  // key() can be lvalue. e.g. this->key() = 3
  // If we do so, there is no need to make k private. So let's just make k public
  //T key; // could be int, could be string, that can be compared by inequality <
  // <- after inserting an element to a skip list, if the key is changed, the order of
  //    the elements can become out of order. So I make key private, and to see the key,
  //    use the following public function.
  const T& get_key() const {return key;}
  // By making the return type const reference, it has the same effect as returning
  // the type itself (cannot become lvalue to change key), and unlike returning the
  // type itself, const reference doesn't copy the return value to another temporary
  // variable of type T (that's what happens when the return type is simply T).

  explicit Skip_list(const T& k, bool is_head=false, double p=1/2.0);
  // In creating a Skip_list object, the user needs to provide the pointer to the head
  // (front) element pointer, since the search/insert/delete operations start from
  // the top left part of a skip list, which is the top pointer of the head element.
  // <- I changed this to providing head element pointer at each operation of
  //    insert/delete/search, so that a skip list element can be reused for another
  //    skip list (a different skip list has distinct head element)
  // To avoid implicit conversion like a single int -> Skip_list(int), I attached "explicit"
  // keyword.

  Skip_list* insert(Skip_list& head);
  const Skip_list* search(const Skip_list& head, const T& k) const;
  Skip_list* erase(Skip_list& head);
  // Since when calling these operation functions, we need a Skip_list object to do these
  // operations on, we don't need a pointer/reference to the Skip_list object to
  // insert/erase. But for search operation, we need a key for which the Skip_list object
  // is searching.
  // In every operation, the reference (not pointer, since I do not accept a nullptr) to the 
  // head Skip_list object is needed, because searching in a skip list starts from the 
  // top left corner of it, which is the top level forward_ptr in the head element.

  // display a skip list for debug
  void display(const Skip_list& head);

  // destructor, since each object acquires a dynamically allocated memory by "new"
  // (and its pointer is stored in forward_ptr) like the vector in ch17, 18
  ~Skip_list(){
    delete[] forward_ptr;
  }

  // copy constructor, as we did in Vector class, to avoid issues mentioned in p 632
  Skip_list(const Skip_list& sl)
    : prob{sl.prob}, forward_ptr{new Skip_list*[sl.height]}, height{sl.height},
      max_height{1000}, is_head{sl.is_head}
  {
    // forward_ptr = new Skip_list*[sl.height];
    // height = sl.height;
    // in ch18, these are initialized in the initializer members as above
    for(int i=0; i<sl.height; ++i)
      forward_ptr[i] = sl.forward_ptr[i];
  }
  // For the same reason as copy constructor, define copy assignment
  Skip_list& operator=(const Skip_list& sl){
    // first, since this Skip_list object might already have other member variables,
    // especially allocated memory for forward_ptr, delete it first
    delete[] forward_ptr;

    // Then, the rest is the same as the above copy constructor
    forward_ptr = new Skip_list*[sl.height];
    height = sl.height;
    is_head = sl.is_head;
    // note: in assignment operator, since the Skip_list object on the left (the object
    // to which the right object is copied) already has its own member states, and since
    // max_height is already set to 1000, we don't need max_height = sl.max_height
    for(int i=0; i<sl.height; ++i)
      forward_ptr[i] = sl.forward_ptr[i];
    // In Vector class in ch18, the author uses std::copy() function

    return *this;
    // This return value lets the called function be capable of doing chain process
    // like (Skip_list 1 = Skip_list 2) = Skip_list 3;, as in istream operation
    // overload. I don't know this is appropriate. We can set the return type void.
    // But since the Vector class defined in ch18 has this kind of return type, I'll
    // follow that convention.
  }

  // move constructor, for the case where a Skip_list object that would soon destroyed
  // is copied to another Skip_list object
  Skip_list(const Skip_list&& sl)
    : height{sl.height}, prob{sl.prob}, forward_ptr{sl.forward_ptr}, max_height{1000},
      is_head{sl.is_head}
  {
    /*
    forward_ptr = sl.forward_ptr;
    // 1st, let this object have the allocated memory for the soon-destroyed object
    prob = sl.prob;
    height = sl.height;
    */ // since this is a kind of constructor, we can initialize the members in the
    // above initializer members

    // then, to avoid the moved allocated memory being destroyed in the destructor
    // of the soon-destroyed object, let sl's forward_ptr have a nullptr
    sl.forward_ptr = nullptr;
    sl.height = 0;
  }
  // Move assignment, for the same reason as move constructor
  Skip_list& operator=(const Skip_list&& sl){
    // the content is the same as that of the above move constructor, except that this
    // case has a return value
    forward_ptr = sl.forward_ptr;
    prob = sl.prob;
    height = sl.height;
    is_head = sl.is_head;

    sl.forward_ptr = nullptr;
    sl.height =  0;

    return *this;
  }

private:
  T key; // could be int, could be string, that can be compared by inequality <
  
  double prob;			// probability of promoting another level
  Skip_list<T>** forward_ptr;
  // pointer to point to an array of pointers to Skip_list. The length of this array is
  // determined by
  int height;
  // decided randomly based on prob, except head element. For head elements, this would
  // become maximum (== flipping a coin and all results are heads)
  // For a head element, this later starts to mean the largest index of forward_ptr
  // that points to non-nullptr object in insert()
  
  int max_height;
  // a cap for the height

  bool is_head;
  // to prevent a head to be inserted to another skip list with another head
};

template<typename T>
Skip_list<T>::Skip_list(const T& k, bool is_h, double p)
  // default arguments are in the declaration
  : key{k}, prob{p}, max_height{1000}, height{1}, is_head{is_h}
{
  if(prob<0 || prob>1)
    error("Error in initializing a Skip_list object. The probability must be in [0,1).");
  
  // if the object is head, make the size of this forward_ptr the maximum
  if(is_head){
    height = max_height;
    // then, to make this head object always come before any elements,

    key = numeric_limits<T>::lowest();
    // For the head's key always become the smallest number when comparing other elements
    // But this works only when T is int, float, or double.
    // When T is other types such as string, the minimum key value would be "" (empty
    // string). But when T is a user-defined type, it's impossible to define the "minimum".
    // Maybe 1 solution could be to let the users set the minimum value when the Skip_list
    // object to be instantiated is a head element, by taking the provided argument k
    // as the head value. But in that case, I (builder of this class) have to wish that
    // they don't provide a nonsense value for head elements, and I'm sure that kind of
    // human errors will happen. I don't know how to deal with that for now. So for now,
    // let's stick to this minimum value only.
  }
  else{
    // based on the prob, decide the height (keep flipping the coin with heads coming up with
    // the prob). The height must be at least 1
    random_device rand_dev;
    // random seed for the following random engine. We can define the random engine without
    // this random_device seed, but in that case, invocations of ./main generate
    // the same random number cycle every time. By using this random seed to the random
    // engine, it can generate different random number sequences in each ./main invocation.
    // By the way, the same behavior is observed when I don't use the following random
    // engine, and set the rand_dev in the place of generator in distribution(...).
    // I don't know the difference between them for now.
    // ref: https://stackoverflow.com/questions/21102105/random-double-c11
    static default_random_engine generator(rand_dev());
    // this line should be static. It seems a random number generator should be called
    // only once in a program, and declaring this static does that. Otherwise, each time
    // this constructor is called, the same random number cycle is repeated.
    // We can define this generator without rand_dev(), but in that case, as I wrote in the
    // comment above, the same random number sequence is used every time ./main is invoked.
    // The same random number sequence is used until main.cpp is re-compiled (sometimes,
    // even after re-compilation, ./main uses the same sequence as before the
    // re-compilation)
    uniform_real_distribution<double> distribution(0.0,1.0);
    // random uniform distribution in range [0,1.0) (not includes 1.0)
    double rn{distribution(generator)}; // random number in [0, 1.0)
    while(rn <= prob && height <= max_height){
      height++;
      rn = distribution(generator);
    }
    
  }
  
  forward_ptr = new Skip_list<T>*[height]{nullptr};
  // set all element's pointer to nullptr

  if(is_head)
    height = 1;
  // to avoid searching empty forward_ptr elements in head, I update head's height in
  // insert(). To do so, head's height needs to be 1
}

// display each layer for debug
template<typename T>
void Skip_list<T>::display(const Skip_list& head){
  const Skip_list* p{&head};
  
  for(int i=0; i<head.height; ++i){
    cout << "level " << i << ": head -> ";
    p = head.forward_ptr[i];
    while(p!=nullptr){
      cout << p->key << " -> ";
      p = p->forward_ptr[i];
    }
    cout << "null\n";
  }
}

// insert "this" element after the element with key less than "this" key, but largest among
// elements with key less than "this"
template<typename T>
Skip_list<T>* Skip_list<T>::insert(Skip_list<T>& head){
  // the search for the element with key largest among those with keys smaller than "this"
  // element, is divided into 2 parts. The 1st part is just searching for it, without
  // changing the forward pointers of existing elements. THe 2nd part is searching with
  // changing the forward pointers of existing elements. The 1st part is for when the
  // searcing levels are higher than the height of "this" element, and the 2nd part is for
  // when they are equal to or lower than the height of "this".

  // sanity check
  if(!head.is_head)
    error("Error in Skip_list::isert(). The argument must be a head element of Skip_list class");
  if(this->is_head)
    error("Error in Skip_list::isert(). The object to be inserted (the one for which this function is called) must not be a head element of Skip_list class");
  
  // to not search upper part of forward_ptr of head that just point to nullptr, update
  // the largest index of head's forward_ptr that points to some object.
  // So every time a new element is inserted, check whether to update it.
  head.height = (height > head.height)? height : head.height;
  
  // const Skip_list<T>* p{&head};
  // Note: if I write this without "const", this won't compile, because non-const pointer
  // may change the values pointed to, and head is declared as const.
  // To make it possible to point to a const object and later change the objects to be
  // pointed to by the pointer, use const pointer, since const pointer cannot change
  // the state of the object it points to, but it can change the address it holds.
  // <- later, I removed const from the argument, to update head's height as above.
  Skip_list<T>* p{&head};
  
  // 1st part
  for(int i=head.height-1; i>height-1; --i){ // 0-indexed, so -1 is needed
    // at level i, keep moving forward (toward the end) until finding nullptr
    // or an element with key larget than "this" key
    while(p->forward_ptr[i]!=nullptr && p->forward_ptr[i]->key < key){
      // If p->forward_ptr[i] is nullptr, the while condition breaks at this 1st condition.
      // So the 2nd condition is not executed, thus we need not worry about trying to
      // access key of nullptr
      p = p->forward_ptr[i];
    }
  }

  // 2nd part
  for(int i=height-1; i>-1; --i){ // 0-indexed, so -1 is needed
    // this while-loop is the same as the one in the 1st part
    while(p->forward_ptr[i]!=nullptr && p->forward_ptr[i]->key < key){
      p = p->forward_ptr[i];
    }
    // p now points to the element right before the position into which "this" is to be
    // inserted.
    // Then, update the forward_ptr of "this" element and the element before "this"
    // element at this level
    forward_ptr[i] = p->forward_ptr[i];
    p->forward_ptr[i] = this;
  }
  
  return this;
  // The type of this in a member function of class X is X* (pointer to X)
  // https://en.cppreference.com/w/cpp/language/this
  // So const is not attached to this.
  // But as in p620, the compiler ensures that the value in this does not change.
}

// search for the element with key equal to the provided argument k, or larget among
// keys less than k.
// The content is almost the same as the 1st part of Skip_list<T>::insert() except the
// indexing and the inequality (< changes to <= to include the key equal to k)
template<typename T>
const Skip_list<T>* Skip_list<T>::search(const Skip_list& head, const T& k) const{
  // Once const is attached to the argument type, it sticks throughout this function,
  // and outside of this function, unless we strip const away by const_cast<Skip_list<T>*>.

  // Since search() operation doesn't change any internal states of searched objects, I
  // don't do sanity check in this function, unlike insert()
  
  const Skip_list* p{&head};

  for(int i=head.height-1; i>-1; --i){
    while(p->forward_ptr[i]!=nullptr && p->forward_ptr[i]->key <= k){
      p = p->forward_ptr[i];
    }
  }
  return p;
}

// remove "this" object from the skip list of the provided head
template<typename T>
Skip_list<T>* Skip_list<T>::erase(Skip_list& head){
  // Once const is attached to the argument type, it sticks throughout this function,
  // and outside of this function, unless we strip const away by const_cast<Skip_list<T>*>.
  // In erase(), I want to change the states of Skip_list objects pointed to by the
  // folloiwng pointer (changing the forward_ptr), so I didn't use const argument
  // in the first place.

  // Since this operation deals with the case where it cannot find the "this" object in
  // this skip list inside the folloiwng for-loop, in this function either, I don't have
  // to do sanity check, unlike insert()
  
  Skip_list* p{&head};
  for(int i=this->height-1; i>-1; --i){ // start searching for "this" object from its height
    while(p->forward_ptr[i] != this && p->forward_ptr[i] != nullptr){
      p = p->forward_ptr[i];
    }
    // since the search for "this" object starts from "this" object's height, the above
    // search must find "this" object at level i. If it cannot find it, that's an error.
    if(p->forward_ptr[i] == nullptr){
      error("In Skip_list::erase(), cannot find the element in this skip list of the provided head.");
    }
    p->forward_ptr[i] = this->forward_ptr[i];
    this->forward_ptr[i] = nullptr; // disconnect "this" object at level i
  }

  return this;
}


int main()
  try{
    Skip_list<int> head(0, true);
    Skip_list<int> sl3(7);
    sl3.insert(head);
    Skip_list<int> sl1(3);
    sl1.insert(head);
    Skip_list<int> sl2(6);
    sl2.insert(head);
    Skip_list<int> sl6(19);
    sl6.insert(head);
    Skip_list<int> sl4(9);
    sl4.insert(head);
    Skip_list<int> sl5(12);
    sl5.insert(head);

    const Skip_list<int>* p;
    p = sl4.search(head, 6);
    cout << p->get_key() << endl;

    sl5.erase(head);
    
    head.display(head);

    return 0;
  }
  catch(exception& e){
    cerr << e.what() << endl;
    return 1;
  }
  catch(...){
    cerr << "Unknown error happens\n";
    return 1;
  }
