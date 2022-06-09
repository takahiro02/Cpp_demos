

#ifndef VECTOR_GUARD
#define VECTOR_GUARD 1

#include<memory>		// for unique_ptr

struct Out_of_range{};	// for throwing the range-checking error in Vector::at()
// There seems to be another out_of_range class in std. So to avoid ambiguity error, I
// capitalize the first letter o.

// To achive RAII, define vector_base (p705, 706)
template<typename T, typename A>
struct vector_base {
  A alloc;			// allocator
  // shouldn't be a reference A&, because allocator<T> takes only 1 byte (because it has no data
  // members, but only has member functions, I think. see My_Allocator in Ex8 on this file),
  // whereas a reference takes 8 bytes (since a reference is internally translated as a pointer).
  T* elem;			// start of allocation
  int sz;			// number of elements
  int space;			// amount of allocated space

  vector_base(const A& a, int n)
    : alloc{a}, elem{alloc.allocate(n)}, sz{n}, space{n}{}
  ~vector_base(){
    //cout << "### destructor of elem= " << elem << ", sz= " << sz << ", space= " << space << endl;
    
    for(int i=0; i<sz; ++i) alloc.destroy(&elem[i]);
    // In the book, he didn't include the destroy() operation in this destructor, but I thik
    // this operation is needed before deallocation.
    // <- But if uninitialized_copy() in a Vector's member throws an exception, in the function,
    //    already constructed elements are destroyed. Thus, if I perform alloc.destroy() here
    //    as well, that could cause double deallocation when the type T contains dynamically
    //    allocated memory, and its destructor deallocates it.
    // <- But if I don't perform destruction of each element, allocated memory in each element
    //    would not be freed, when the program runs without exception.
    // -> Then, I will use try-catch block for each uninitialized_copy() in Vector, and in the
    //    catch(){} block, I can set sz to 0, so that in this destructor, no destruction happens.
    alloc.deallocate(elem, space);
  }
};

// This Vector class is first copied from ch18/try_this/main.cpp
// Then it is modified according to ch19's content (p706: vector_base)
template<typename T, typename A = allocator<T>> // allocator: p691
class Vector : private vector_base<T,A> {
  // int sz;			// the size
  // T* elem;			// a pointer to the element
  // int space;			// from p672. Store an amount of free space reserved
  // A alloc;			// use allocator to handle memory for elements (p691)
  // In my understanding, "allocator" deals with objects without destructor that may
  // dynamically allocate memory in each of the objects. So allocator somehow destroies
  // all dynamically allocated memory even when its destructor is not defined?
  // <- No, it doesn't. An allocator explicitly calls destructor by alloc.destroy(). So what
  //    it does is to separate allocate/construct into 2 steps, and destroy/deallocate into
  //    2 steps, unlike new and delete, where these 2 steps are done at once (in "new", a
  //    default constructor is always called).

  // On page 706, we put the representation of vector into vector_base class
   A Alloc;			// I think the derived Vector class also needs to have
  // allocator, because vector_base class uses a reference to an allocator in its constructor,
  // that means the allocator must exist somewhere beforehand.
public:
  // string label;			// for debug
  
  int size() const {return this->sz;}	// p605

  // default constructor (p672)
  Vector()
    // : sz{0}, elem{nullptr}, space{0}
    : vector_base<T,A>(Alloc, 0)
  {}

  // from p673
  void reserve(int);

  // from p706, the version of reserve which uses vector_base for holding newly allocated
  // memory, to release it in case an exception is thrown in the middle of the function
  void reserve2(int);

  // from p673
  int capacity() const {return this->space;}

  // from p674, modified by p690
  void resize(int newsize, T val = T());

  // from p674-675
  void push_back(const T& d);
  
  explicit Vector(int s, T def = T())	// explicit keyword: p643
    // : sz{s}, elem{new T[s]}, space{s}
      // Since in here "new" is used, as well as allocating the s elements of type T, it applies
      // default constructor to each of the elements at the same time.
      // I modified it based on p692, so that it can take some specified constructor for each
      // allocated element.
    // : sz{s}, space{s}
    : vector_base<T,A>(Alloc, s) // modified from p706
  {
    // for debug
    // cout << "### explicit Vector(int s, T def = T()) is called\n";
    
    /*
    for(int i=0; i<sz; ++i)
      elem[i] = def;		// initialize
    */

    // separate allocation and construction of elements, so that a class T without default
    // constructor can be created with a specified (by the 2nd argument) constructor.
    // From p692
    //this->elem = this->alloc.allocate(s);	// no initialization happens
    // This allocation has already happened in the constructor of vector_base<T,A>
    try{
      for(int i=0; i<s; ++i)
	this->alloc.construct(&this->elem[i], def);
    }
    catch(...){
      //this->alloc.deallocate(this->elem, this->space);
      // <- after RAII is achieved via using vector_base, I shouldn't (and must not) deallocate
      //    in catch(){} block, since at this stage, vector_base is already constructed, thus,
      //    once uninitialized_copy() throws an exception and this Vector object gets out of
      //    scope, vector_base's destructor is called, and there, deallocation happens.

      this->sz = 0;
      // Since in uninitialized_copy(), destruction of already constructed elements are already
      // done, to avoid destroying the same elements again in vector_base's destructor, I set
      // sz to 0
      
      throw;			// throw the same exception to an upper scope again
    }
    // Referencing Vector(initializer_list<T> lst), for the case of alloc.construct() throws
    // an exception, I set the for-loop into try-catch block, because if an exception is
    // thrown in a constructor, the corresponding destructor is not called even when the
    // object gets out of scope.

    // Notice: if we use allocator here, we need to use allocator in destroying and deallocating
    //         the elements in destructor too. Otherwise, errors such as "error for object
    //         0x6000012504b8: pointer being freed was not allocated" is generated. So be sure
    //         to make pairs of new <-> delete, and (allocator.allocate(), allocator.construct())
    //         <-> (allocator.destroy(), allocator.deallocate()).
    //         <- But when I used int, as T, I can mix "new" and "allocator.destroy()",
    //            "allocator.deallocate()", whereas when I used No_default class defined in
    //            main(), I cannot mix them. I guess maybe when using built-in types, mixing
    //            them won't produce any error, but when using user-defined types, it's
    //            prohibited. I don't know why this behavior happens.
  }

  Vector(initializer_list<T> lst)
    // : sz{static_cast<int>(lst.size())}, space{sz}, elem{new T[sz]}
    // : sz{static_cast<int>(lst.size())}, space{sz} // use allocator instead of new
    : vector_base<T,A>(Alloc, static_cast<int>(lst.size()))
  {
    //cout << "### Vector(initializer_list<T> lst) is called\n";
    
    // copy(lst.begin(), lst.end(), elem);

    this->elem = this->alloc.allocate(this->space);
    // for(int i=0; i<sz; ++i)
    //   alloc.construct(&elem[i], lst.begin()[i]);
    // allocator.construct() uses placement new, and placement new uses copy constructor of
    // type T. Thus, if T allocates memory, and T doesn't have a proper copy constructor like
    // the one of Vector, it can cause deallocation twice on the same address, which causes
    // error like "error for object 0x600000b101c0: pointer being freed was not allocated".
    // For allocator.construct(), https://en.cppreference.com/w/cpp/memory/allocator/construct
    // For placement new, https://www.geeksforgeeks.org/placement-new-operator-cpp/

    try{
      uninitialized_copy(lst.begin(), lst.end(), this->elem);
    // From p706. Rather than alloc.construct(), it doesn't need to use for-loop, and when
    // an exception is thrown in the middle of copy construction, already constructed elements
    // in elem are destroyed (in an unspecified order).
    // But I think if such a situation happens where an exception is thrown in the middle,
    // the same elements are to be destroyed twice (once in uninitialized_copy(), and 2nd in
    // the destructor of Vector), which can cause memory deallocation error if type T has
    // allocated memory, and destructor that deallocate the allocated memory.
    // So I decided to put this function in try{}catch(){} block
    // <- It turned out that when an exception is thrown in a constructor, the corresponding
    //    destructor is not called even when the variable gets out of scope. This is I think
    //    because as explained in p701, for a destructor to be called, its constructor has to
    //    be completed.
    // <- This means, I think, the allocated memory in this constructor will not be freed
    //    until the end of the program. So I need to free the allocated memory explicitly
    //    in this try-catch block.
    }
    catch(...){
      // cerr << "uninitialized_copy()'s exception is caught\n";
      // cerr << e.what() << endl;
      //   this->alloc.deallocate(this->elem, this->space);
      // <- after RAII is achieved via using vector_base, I shouldn't (and must not) deallocate
      //    in catch(){} block, since at this stage, vector_base is already constructed, thus,
      //    once uninitialized_copy() throws an exception and this Vector object gets out of
      //    scope, vector_base's destructor is called, and there, deallocation happens.

      this->sz = 0;
      // Since in uninitialized_copy(), destruction of already constructed elements are already
      // done, to avoid destroying the same elements again in vector_base's destructor, I set
      // sz to 0
      
      throw;			// throw the same exception to an upper scope again
    }
    
    // cout << "### end of Vector(initializer_list<T> lst)\n";
    
    // uninitialized_copy(lst.begin(), lst.end(), this->elem); // copy
    // https://en.cppreference.com/w/cpp/memory/uninitialized_copy
    // Copies elements from the range [first, last). And since lst.end() refers to
    // one past the last element, the element referred to by lst.end() not being included
    // is OK.
  }

  Vector(const Vector&);	// copy constructor
  Vector& operator=(const Vector&); // copy assignment
  // const assignment doesn't exist, because if a const Vector gets some assignment by =,
  // the error is detected by the compiler. And assigned Vector doesn't come right hand side?
  // e.g. v1 = (v2 = v3); <- at this point, v2 is non-const vector, if this is valid.
  // By the way, if operator=(Vector, Vector) is defined outside of the Vector class, in addition
  // to this operator=(const Vector&) inside Vector class, I wonder in such a situation like
  // the example, which operator overload is called.

  Vector(Vector&& a);		// move constructor
  Vector& operator=(Vector&&);	// move assignment
  
  ~Vector(){
    // cout << "### " << label << "'s ~Vector() is called.\n";
    // cout << "### elem size = " << sz << ", space = " << space << endl;

    //delete[] elem;

    // for(int i=0; i<sz; ++i) alloc.destroy(&elem[i]);
    // // don't forget to destroy elements before deallocating them
    // alloc.deallocate(elem, space);
    // <- from chapter 19.5.6 (p705), I let the functionality of destructor have the base
    //    class, vector_base

    // Notice: When allocating and construction dynamic heap memory with "new", we have to
    //         destroy and deallocate them with "delete". On the other hand, if we allocate
    //         and construct with allocator.allocate() and allocator.construct(), we have to
    //         use allocator.destroy() and allocator.deallocate() to destroy and deallocate
    //         them. If, we use "new", but try to deallocate with allocator.deallocate(),
    //         "error for object 0x6000012504b8: pointer being freed was not allocated" error
    //         is generated.
    //         <- Mysteriously, sometimes, this isn't true. When I allocate memory with "new"
    //            in initializer list constructor Vector(initializer_list<T> lst), and
    //            destroying it with alloc.destroy() and deallocate the memory with
    //            alloc.deallocate(), it compiled, and ran without problem. I am confused about
    //            the inconsistent behavior. Another inconsistent behavior is when I create
    //            Vector by Vector<int> vi2(3); and allocate memory with allocator with
    //            vi2.reserve(10);, the allocated memory seemed to be safely destroyed and
    //            deallocated with delete[] elem;.
    //         <- I think the difference between the case where "new" and "delete[]" are mixed
    //            with alloc.destroy() and alloc.deallocate() and the case where they can't is
    //            whether the type T is built-in type or user-defined type. When they can be
    //            mixed above, the used type is int, but when I used No_default type, with the
    //            same operations (Vector<No_default> vnd(3); vnd.reserve(10);), the error
    //            "error for object 0x600003770468: pointer being freed was not allocated" is
    //            generated. So for consistency, I think it's best to use allocator for all
    //            member functions, to avoid such errors when the user uses user-defined classes.
    //            Maybe this behavior is implementation-dependent (= under different compilers
    //            the program behaves differently).

    // From p706, the work of destruction and deallocation is done in the base class,
    // vector_base's destructor. So here we don't have to do anything.
  }
  T& operator[](int n){return this->elem[n];}
  T operator[](int n) const {return this->elem[n];}

  // range-checking access at() (p693-694)  
  T& at(int n){
    if(n<0 || this->sz<=n) throw Out_of_range();
    return this->elem[n];
  }
  const T& at(int n) const {
    if(n<0 || this->sz<=n) throw Out_of_range();
    return this->elem[n];    
  }
};

// In the copy constructor, unlike copy assignment, since there exists no Vector object
// to be assigned to previously, we don't have to care about the optimization done in
// the copy assignment below. I just added space{arg.space} to the previous copy constructor.
template<typename T, typename A>
Vector<T,A>::Vector(const Vector& arg) // from p633
  // : sz{arg.sz}, elem{new T[arg.sz]}, space{arg.space}
// rewrite copy constructor with allocator
  // : sz{arg.sz}, space{arg.space}
  : vector_base<T,A>(Alloc, arg.sz)
{  
  // copy(arg.elem, arg.elem + arg.sz -1, elem);
  // what is arg.elem.sz? does double* have a member sz? I don't think so.
  // I think the correct syntax is (arg.elem + arg.sz -1) to show the address of
  // the last element

  // this->elem has already been allocated with arg.sz 
  try{
    // for(int i=0; i<this->sz; ++i) this->alloc.construct(&this->elem[i], arg.elem[i]);
    // Since when using alloc.construct(), if some iteration throws an exception, the elements
    // that are already created are not destroyed, it's better to use uninitialized_copy()
    // (I can do without the function, by remembering the iteration count at which
    // alloc.construct() throws an exception, and explicitly destroying the already constructed
    // elements in the catch(){} block below. But using the function is more compact)
    
    uninitialized_copy(arg.elem, &arg.elem[arg.sz], this->elem);
  }
  catch(...){
    // this->alloc.deallocate(this->elem, this->space);
    // <- after RAII is achieved via using vector_base, I shouldn't (and must not) deallocate
    //    in catch(){} block, since at this stage, vector_base is already constructed, thus,
    //    once uninitialized_copy() throws an exception and this Vector object gets out of
    //    scope, vector_base's destructor is called, and there, deallocation happens.

    this->sz = 0;
    // Since in uninitialized_copy(), destruction of already constructed elements are already
    // done, to avoid destroying the same elements again in vector_base's destructor, I set
    // sz to 0
    
    throw;			// throw the same exception to an upper scope again
  }
  // Referencing Vector(initializer_list<T> lst), for the case of alloc.construct() throws
  // an exception, I set the for-loop into try-catch block, because if an exception is
  // thrown in a constructor, the corresponding destructor is not called even when the
  // object gets out of scope.
}
// modified wit p676-677
template<typename T, typename A>
Vector<T,A>& Vector<T,A>::operator=(const Vector& a){ // from p635
  if(this==&a) return *this;		    // self-assignment, no work needed

  if(a.sz<=this->space){		// enough space, no need for new allocation
    for(int i=0; i<a.sz; ++i) this->elem[i] = a.elem[i]; // copy elements
    this->sz = a.sz;
    return *this;
  }
  
  // when there is no enough space for copying a to this Vector
  // T* p = new T[a.sz];
  // copy(a.elem, a.elem + a.sz -1, p);
  // // again, I think a.elem.sz should be (a.elem + a.sz -1)
  // delete[] elem;

  // rewrite it with allocator, so that I don't use a mixture of "new" and "alloc.deallocate()"
  // T* p = alloc.allocate(a.sz);
  unique_ptr<T> p{this->alloc.allocate(a.sz)}; // from p703
  for(int i=0; i<a.sz; ++i) this->alloc.construct(&p[i], a.elem[i]);
  // Notice: since construction on p is based on a, the iteration size is a.sz, but since
  //         destrution is based on the previous allocated memory elem, its iteration size
  //         is sz, and deallocation is done with this->space
  for(int i=0; i<this->sz; ++i) this->alloc.destruct(&this->elem[i]);
  this->alloc.deallocate(this->elem, this->space);

  this->space = this->sz = a.sz;		// means space = (sz = a.sz);
  // elem = p;
  this->elem = p.release();
  // from p703, to prevent the allocated memory from being dealocated
  
  return *this;
}
template<typename T, typename A>
Vector<T,A>::Vector(Vector&& a)	// from p639
  // : sz{a.sz}, elem{a.elem}, space{a.space}
  : vector_base<T,A>(Alloc, a.sz) // rewrite with the idea of chapter 19.5.6 (p705-706)
{
  // copy the elements in a with the idea of chapter 19.5.6 (p705-706)
  uninitialized_copy(a.elem, &a.elem[a.sz], this->elem);
  
  a.space = 0;
  a.sz = 0;
  a.elem = nullptr;
}
template<typename T, typename A>
// modified with p676-677
Vector<T,A>& Vector<T,A>::operator=(Vector&& a){ // from p639
  // Since in this move assignment, there should be no case where a and the assigned Vector
  // (this) are the same object (because if this move assignment is called, that means variable
  // a is about to end its lifetime, whereas "this" Vector persists).
  // Thus, case of if(this==&a) is not needed, unlike the above copy assignment.
  
  if(a.sz<=this->space){		// enough space, no need for new allocation
    for(int i=0; i<a.sz; ++i) this->elem[i] = a.elem[i]; // copy elements
    this->sz = a.sz;
    return *this;
  }
  // This case doesn't use the concept of the move assignment, because the concept of move
  // assignments is to re-use dynamically allocated memory of a local object, which would
  // be destroyed otherwise. But in this case (if(a.sz<=space)), since a.elem is not set to
  // nullptr, a's allocated memory is destroyed as opposed to the original move assignment
  // in p639. But I think this is valid, to keep the rule (?) of the reserved memory not
  // decreasing.
  // If the rule can be ignored, and reserved memory can shrink, this case won't make sense.
  // In such a case where you want to re-use the dynamically allocated memory in a local
  // environment, you can just delete this case.
  
  // delete[] elem;

  // rewrite delete[] elem; to use allocator's deallocate, to avoid mixtures of alloc.allocate()
  // and delete (see my comments in ~Vector())
  for(int i=0; i<this->sz; ++i) this->alloc.destroy(&this->elem[i]);
  this->alloc.deallocate(this->elem, this->space);

  this->elem = a.elem;
  this->sz = a.sz;
  this->space = a.space;
  a.elem = nullptr;
  a.sz = 0;
  a.space = 0;
  return *this;
}

// from p673
// modified from p692 (allocator)
// again modified from p706 (using vector_base<T,A> to achieve RAII)
template<typename T, typename A>
void Vector<T,A>::reserve(int newalloc){
  // cout << "### Vector::reserve() is called\n"; // for debug
  
  if(newalloc <= this->space)
    return;			// never decrease allocation
  
  // T *p = new T[newalloc];
  // for(int i=0; i<sz; ++i) p[i] = elem[i];
  // delete[] elem;

  // T* p = alloc.allocate(newalloc); // allocate new space
  unique_ptr<T[]> p{this->alloc.allocate(newalloc)}; // from p703 (this is try-this p707)
  // unique_ptr<T> is basically for only 1 object. It is not for an array of objects. That's
  // why it doesn't provide any subscript [] operator, or + operator (usually pointer+i gives
  // the address i objects forward from pointer). So, it deallocates the memory with delete,
  // usually. If we want to use it for an array, we can say unique_ptr<T[]> (notice the square
  // brackes). That way, the deallocation happens with delete[]. And if we use T[] instead of
  // T in unique_ptr<>, then the subscript operator becomes available (+ operator is till
  // not available)
  // https://stackoverflow.com/questions/8940931/
  for(int i=0; i<this->sz; ++i) this->alloc.construct(&p[i], this->elem[i]); // copy
  // Since unique_ptr<T> doesn't provide subscripting [] or forwarding + operator, we have
  // to get the address i objects forward with p.get()+i, instead of &p[i] or p+i.
  // But if we use unique_ptr<T[]>, then the subscripting operator becomes available.
  for(int i=0; i<this->sz; ++i) this->alloc.destroy(&this->elem[i]);	   // destroy
  this->alloc.deallocate(this->elem, this->space);		   // deallocate old space
  
  // elem = p;
  this->elem = p.release();		// from p703 (this is try-this p707)
  this->space = newalloc;
}

// From p706, the version of reserve() that uses vector_base for holding a newly allocated
// memory, to achieve RAII for the allocation
template<typename T, typename A>
void Vector<T,A>::reserve2(int newalloc){  
  if(newalloc <= this->space)
    return;			// never decrease allocation
  
  vector_base<T,A> b{this->alloc, newalloc};		     // allocate new space

  // For the case where uninitialized_copy() throws an exception, I added try-catch block,
  // as in Vector<T,A>::Vector(const Vector& arg), Vector(initializer_list<T> lst), and
  // Vector(int s, T def = T()).
  try{
    //uninitialized_copy(b.elem, &b.elem[this->sz], this->elem); // copy
    // I think the code in p706 is wrong. The copy is done from the range of the first 2
    // arguments to the 3rd argument address
    
    // https://en.cppreference.com/w/cpp/memory/uninitialized_copy
    uninitialized_copy(this->elem, &(this->elem[this->sz]), b.elem); // copy
    // this->elem to b.elem
  }
  catch(...){
    b.sz = 0;
    // Since in uninitialized_copy(), destruction of already constructed elements are already
    // done, to avoid destroying the same elements again in vector_base's destructor, I set
    // sz to 0
    // For "this" object, the elements are still not destroyed, and they are managed by "this"
    // object's destructor. In this catch(){} block, I manage the destruction of the locally
    // allocated memory b.
    
    throw;			// throw the same exception to an upper scope again
  }

  // for(int i=0; i<this->sz; ++i)
  //   this->alloc.destroy(&this->elem[i]);	// destroy old allocation
  // destruction is done in vector_base's destructor, so I commented this out
  
  //swap<vector_base<T,A>>(*this, b);		// swap representations
  // https://www.cplusplus.com/reference/algorithm/swap/
  //cout << "### swap<vector_base<T,A>>() has finished\n";
  // <- using swap<>() makes the error of trying to deallocate the same address twice, because
  //    in swap(), it prepares 1 local variable copying *this. And after *this and b are swapped,
  //    the local variable in swap is destroyed, and at that moment, calls destructor of
  //    vector_base<T,A>, which deallocates old memory.
  //    And when this reserve2() ends, its local variable, b, is destroyed, and at that moment,
  //    the already destroyed old memory tries to be destroyed again (because at the time,
  //    b points to the old memory), which causes double deallocation, and that's the cause of
  //    the error.
  //    (also, it seems even when a reference is out of scope, its referenced destructor is not
  //     called)
  
  // the following 4 lines except the comments are what happens in
  // swap<vector_base<T,A>>(*this, b)
  /*
  vector_base<T,A>& a(*this);
  vector_base<T,A> temp(a);
  cout << "### a.elem= " << a.elem << ", a.sz= " << a.sz << ", a.space= " << a.space << endl;
  cout << "### b.elem= " << b.elem << ", b.sz= " << b.sz << ", b.space= " << b.space << endl;
  a = b;
  b = temp;
  cout << "### alfter swapping:\n";
  cout << "### a.elem= " << a.elem << ", a.sz= " << a.sz << ", a.space= " << a.space << endl;
  cout << "### b.elem= " << b.elem << ", b.sz= " << b.sz << ", b.space= " << b.space << endl;
  */
  
  // the following code fixes the above problems
  T* tp{this->elem};
  int tspace{this->space}, tsz{this->sz};
  
  this->elem = b.elem;
  this->space = b.space;
  // this->vec_data->sz stays the same

  // to properly destroy and deallocate the old memory in b's destructor,
  // assign the old information to b
  b.elem = tp, b.space = tspace, b.sz = tsz;
}

// from p674
// The difference of this function from Vector::reserve() is that this function initializes
// the reserved, but free space with val (2nd argument), and changes sz to the newsize, 
// whereas reserve() doesn't initialize the free, reserved elements, or change only variable
// "space", not sz
template<typename T, typename A>
void Vector<T,A>::resize(int newsize, T val){ // default value (p690)
  reserve(newsize);

  // for(int i=sz; i<newsize; ++i) elem[i] = def;

  // modified from p692-693
  for(int i=this->sz; i<newsize; ++i) this->alloc.construct(&this->elem[i], val);
  for(int i=newsize; i<this->sz; ++i) this->alloc.destroy(&this->elem[i]);
  // The latter for-loop is only for when newsize < sz. In that case, the previously stuffed
  // elements are in elem[newsize] ~ elem[sz-1], so we need to call a destructor for these
  // elements. See my note for the diagram.
  
  this->sz = newsize;
}
// If newsize is smaller than the current size, sz shrinks, without changing variable "space".
// This is the case even for newsize == 0, and for newsize < 0.
// When newsize is <0, e.g. -10, sz becomes -10 as well, without emitting an error message.
// When newsize <= sz, any initialization won't happen, since the initialization for-loop
// won't run even one time.
// I think to avoid sz being negative, we need to set an if-condition before "sz = newsize;"

// from p674-675
template<typename T, typename A>
void Vector<T,A>::push_back(const T& val){
  if(this->space==0)
    reserve(8);
  else if(this->sz==this->space)
    reserve(2*this->space);

  // elem[sz] = d;

  this->alloc.construct(&this->elem[this->sz],val); // modified from p692
  
  ++this->sz;
  // sz points to 1 element beyond the last element
}


#endif // VECTOR_GUARD
