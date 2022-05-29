
/* Ex 16 ch19 */
/* basically the code is copied from Vector<T> and vector_base<T> in main.cpp */
/* and the idea is inspired from */
/* https://github.com/bewuethr/stroustrup-ppp/blob/main/chapter19/chapter19_ex16.cpp */

#ifndef VECTOR3_GUARD
#define VECTOR3_GUARD

#include<memory>		/* for allocator<T> and unique_ptr<T> */
#include <stdexcept>      // for std::out_of_range exception in Vector3<T>::at()

using namespace std;

template<typename T, typename A = allocator<T>>
  struct vector_data {
    A alloc;
    int sz;
    int space;
    T* elem;

  // default constructor is prepared for Vector3<T>::reserve2()
  vector_data()
    : sz{0}, space{0}, elem{nullptr}
  {}
  
    explicit vector_data(int s, T def = T())
      : sz{s}, space{s}, elem{alloc.allocate(s)}
    {
      // separate allocation and construction of elements, so that a class T without default
      // constructor can be created with a specified (by the 2nd argument) constructor.
      int i{0};
      /* to know until which element the construction is performed, I define i outside of
         the try{} block, in case an exception happens in the fo-loop constructions */
      try{
	for(i=0; i<s; ++i) alloc.construct(&elem[i], def);
      }
      catch(...){
	sz = i;
	/* if an exception happens in the middle of the for-loop of constructions, elements were 
	   constructed up to element[i-1], so in order to destroy up to elem[i-1] in 
	   ~vector_data(), set sz to i */
	this->~vector_data();	/* explicit call of destructor, because if exception happens
				   in a constructor, its destructor is never called */
	throw;			// throw the same exception to an upper scope again
      }
      // Referencing Vector(initializer_list<T> lst), for the case of alloc.construct() throws
      // an exception, I set the for-loop into try-catch block, because if an exception is
      // thrown in aconstructor, the corresponding destructor is not called even when the
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

    vector_data(initializer_list<T> lst)
      : sz{static_cast<int>(lst.size())}, space{sz}, elem{alloc.allocate(sz)}
    {      
      /* then construct elems */
      /* In case uninitialized_copy() throws an error, I enclose it with try{} block */
      try{
	uninitialized_copy(lst.begin(), lst.end(), elem);
	/* copy the range [lst.begin(),lst.end) to elem */
	/* since lst.end() refers to one past the last element, the element referred to by 
	   lst.end() not being included is OK. */
      }
      catch(...){
	this->sz = 0;
	// Since in uninitialized_copy(), destruction of already constructed elements are already
	// done, to avoid destroying the same elements again in vector_data's destructor, I set
	// sz to 0
	this->~vector_data();	   /* explicit call of destructor, because if exception happens
				   in a constructor, its destructor is never called */
	// I don't know why, but without this->, gcc compiler emitted an error
	throw;			// throw the same exception to an upper scope again
      }
    }

    /* copy constructor */
    vector_data(const vector_data& arg)
      : sz{arg.sz}, space{arg.space}, elem{alloc.allocate(arg.space)}
    {
      try{
	uninitialized_copy(arg.elem, &arg.elem[arg.sz], this->elem);
      }
      catch(...){
	this->sz = 0;
	// Since in uninitialized_copy(), destruction of already constructed elements are already
	// done, to avoid destroying the same elements again in vector_base's destructor, I set
	// sz to 0
	this->~vector_data();		/* explicit call of destructor, because if exception happens
				   in a constructor, its destructor is never called */
	throw;			// throw the same exception to an upper scope again
      }
    }
    
    ~vector_data(){
      /* constructed elements are up to elem[sz-1] */
      for(int i=0; i<sz; ++i) alloc.destroy(&elem[i]);
      alloc.deallocate(elem, space);

      // test if delete[] works
      //delete[] elem;
      // Again, as I noted in the above "Notice" comment, if I use user-defined complex types
      // such as No_default class defined in main.cpp, using delete[] caused segmentation
      // fault (, which is a little surprising, because before, it caused an error like "error
      // for object 0x6000012504b8...")
    }
  };

template<typename T> // allocator: p691
class Vector3
  {
  private:
  vector_data<T>* vec_data;	/* pointers of all types have 8 bytes in size  */
  
  public:
  
  Vector3()
    // : sz{0}, elem{nullptr}, space{0}
  : vec_data{nullptr}
  /* this nullptr is THE advantage we try to get out of Vector3. When this Vector3 is empty, */
  /* it doesn't have even its data representation, so only consumes 8 bytes for vec_data */
  {}
  
  explicit Vector3(int s, T def = T())	// explicit keyword: p643
    // : sz{s}, elem{new T[s]}, space{s}
      // Since in here "new" is used, as well as allocating the s elements of type T, it applies
      // default constructor to each of the elements at the same time.
    : vec_data{new vector_data<T>(s, def)}
  /* using vector_data<T>'s constructor, s elements are allocated and constructed */
      // if I use curly braces {s, def} instead of round braces (s, def) above, the different
      // constructor with initializer_list constructor is called
  {}

  Vector3(initializer_list<T> lst)
  : vec_data{new vector_data<T>(lst)}
  /* Like  Vector3(int s, T def = T()), the actual allocation happens in vector_data's 
     corresponding constructor */
  {}

    // copy constructor
  Vector3(const Vector3& arg)
  : vec_data{new vector_data<T>(*arg.vec_data)}
  {}
  Vector3& operator=(const Vector3&); // copy assignment

  // move constructor (this object takes the resources of a before a is deleted)
  Vector3(Vector3&& a)
  : vec_data{a.vec_data}
  /* unlike move assignment operator, I must not destroy and deallocate the originally allocated
     memory, because this is constructor, meaning this object doesn't have its allocation
     before this */
  {    
    /* to prevent the moved object from being destroyed, set nullptr to a.vec_data */
    a.vec_data = nullptr;
  }
  Vector3& operator=(Vector3&&);	// move assignment
  
  ~Vector3()
  {
    /* 
       vec_data can be nullptr, as in calling default (empty) constructor, or calling move
       constructor. nullptr->something causes segmentation fault. So check if vec_data does 
       point to some object 
    */
    if(vec_data)
      vec_data->~vector_data();
    /* destruction and deallocation of allocated elements are done in vector_data's destructor */
    /* I think since the member is not the object of vector_data itself, but a pointer to it,
       without explicitly calling the pointed object's destructor, the destructor will not be
       called */
  }

    int size() const {
      // It's possible that a user make an empty Vector3<T>, and try to call this function.
      // In that case, vec_data-> causes segmentation fault, since vec_data is still nullptr.
      // So we need additional check
      if(vec_data == nullptr) return 0;
      
      return vec_data->sz;}
    
    int capacity() const {
      // It's possible that a user make an empty Vector3<T>, and try to call this function.
      // In that case, vec_data-> causes segmentation fault, since vec_data is still nullptr.
      // So we need additional check
      if(vec_data == nullptr) return 0;
      
      return vec_data->space;}

  /* version of using unique_ptr<T> */
  void reserve(int);

  // from p706, the version of reserve which uses vector_base for holding newly allocated
  // memory, to release it in case an exception is thrown in the middle of the function
  void reserve2(int);

  void resize(int newsize, T val = T());
  
  void push_back(const T& d);

    T& operator[](int n){return this->vec_data->elem[n];}
    T operator[](int n) const {return this->vec_data->elem[n];}

    // range-checking access at() (p693-694)  
    T& at(int n){
      // It's possible that a user make an empty Vector3<T>, and try to call this function.
      // In that case, vec_data-> causes segmentation fault, since vec_data is still nullptr.
      // So we need additional check
      if(vec_data == nullptr) throw runtime_error("This vector is empty");
      
      if(n<0 || this->vec_data->sz<=n) throw out_of_range();
      // 
      return this->vec_data->elem[n];
    }
    const T& at(int n) const {
      // It's possible that a user make an empty Vector3<T>, and try to call this function.
      // In that case, vec_data-> causes segmentation fault, since vec_data is still nullptr.
      // So we need additional check
      if(vec_data == nullptr) throw runtime_error("This vector is empty");
      
      if(n<0 || this->vec_data->sz<=n) throw out_of_range();
      return this->vec_data->elem[n];    
    }
  };

#include "vector3.cpp"
// When making the template declaration and definition separate in a header and another file,
// I need to #include (== copy and paste) the definition in the end of the header file.
// This is the same as including the template definitions inside the header file.
// Because templates are not an actual class, but a frame of a class from which actual classes
// can be made through instantiation. Thus, without this #include, when linking object files,
// the actual class cannot be found, thus it causes link error.
// Vector3<T> is used in main.cpp, thus, actual classes are made in compiling main.cpp, not
// in compiling vector3.cpp. At that time of compiling main.cpp, the compiler has to see all
// the definitions and declarations of the template. Thus, we need to include all template
// definitions inside the header file.

// Another solution is when compiling vector3.cpp, the compiler instantiates the actual classes
// by explicitly specifying actual types in vector3.cpp, like
// template class Vector3<int>;
// See https://stackoverflow.com/questions/495021/
// and https://isocpp.org/wiki/faq/templates#templates-defn-vs-decl

// If in different compilation units, a template is instantiated for the same type in each
// cimpilation unit, it's possible that in linkage stage, the linker emits linker error, or
// it's also possible that the linker drops one of the definitions, and links safely. That is
// dependent on the linker setting.

#endif // VECTOR3_GUARD
