
#include "std_lib_facilities.h"
#include "Vector.h"

class Int {
  int i;
public:
  Int(): i{} {}
  Int(int in) : i{in} {}
  Int operator=(int in){i = in; return i;}
  Int operator=(Int I){i = I.i; return i;}
  // private member of another object of the same class can be called in member functions of
  // that class

  Int operator+(Int I){ return i+I.i;} // "this" Int + Int
  Int operator-(Int I){ return i-I.i;}
  Int operator*(Int I){ return i*I.i;}
  Int operator/(Int I){ return i/I.i;}

  int get() const {return i;}
  // If the return type is reference, not int (a temporary copy), we can use this get() at
  // a lvalue.
  // If the return type is int&, we cannot attach "const" qualifier for the function, since it
  // can modify member "i" by setting get() at a lvalue.
};
// Since all the member functions of a class have a fixed first argument Int* this, if we want to
// define io operators that take io objects (e.g. cin, cout) as its first argument, we need to
// define them as stand-alone functions, rather than Int's member functions
istream& operator>>(istream& is, Int& in){
  int i;
  if(!(is >> i)){
    return is;			// fail to read, so is is already set to fail state
  }
  in = i;			// I define assignment operator of Int = (built-in) int
  return is;
}
ostream& operator<<(ostream& os, const Int& in){
  os << in.get();
  return os;
}

// ==============================================================================================

template<typename T>
class My_Allocator {
  // T* front_ptr;
  // I don't think we need to store the address because addresses are passed to each member
  // function. But for precaution.
public:
  // no constructor or destructor is defined.
  
  T* allocate(int element_num);
  void construct(T* mem_ptr, const T& val);
  void destroy(T* mem_ptr);
  void deallocate(T* front_ptr, int element_num);
};
template<typename T>
T* My_Allocator<T>::allocate(int elem_num){
  if(elem_num < 0) throw runtime_error("Negative number of elements is specified in My_Allocator<T>::allocate(int)");
  //front_ptr = malloc(elem_num*sizeof(T));
  // no initialization happens at this point, just keep that bytes of memory in the heap, and
  // return the front address (malloc() doesn't call T's constructors)
  return (T*) malloc(elem_num*sizeof(T));
  // In C, I didn't need this explicit cast, but in C++, without (T*), malloc()'s return type
  // stays (void *), and the compiler emitted an error.
}
// with placement new and copy constructor of type T, copy val to mem_ptr
template<typename T>
void My_Allocator<T>::construct(T* mem_ptr, const T& val){
  new (mem_ptr) T(val);		// placement new, with copy
  // in https://en.cppreference.com/w/cpp/memory/allocator/construct, it uses
  // (void *) to convert mem_ptr to void pointer, but I don't think that's necessary
  
  // Aside: As I learned in Exercise 12 in File_handle::fread(), variables of any address types
  //  have the same kind of information: the address of the front byte of the variable it points
  //  to. So for example, if it is int* x, x points to an int object. Since sizeof(int) == 4,
  //  x has 4 bytes. But what x is really pointing to is the 1st byte (byte for lower bits, like
  //  2^0, 2^1, ..., 2^7) of the 4 bytes. The same is true of a struct. If a struct has 2 ints,
  //  struct X {int x1; int x2;};, then, sizeof(X) == 8 bytes (=2 ints). Then, X* x; holds
  //  the address of the 1st byte of the block of 8 bytes. If all pointer types essentially
  //  hold the same information (address of 1 byte), then, what's the point of differentiating
  //  different pointer types, such as int* and X*? The point is to give arithmetic operations
  //  to the pointer. For example, in int *x;, x+1 or &x[1] gives the address of the 1st byte
  //  of the next 4 bytes (so +1 here means to forward the address by 4 bytes). Or, in X* x;,
  //  x+1 (or &x[1]) gives the 1st byte of the next 8 bytes (so +1 here means to forward the
  //  address by 8 bytes). The compiler can know how many bytes it must forward in +1 operation
  //  to those pointers, because we specify what type of pointer they are.
  //  In contrast, void *x; cannot know by how many bytes the compiler must forward the address,
  //  although x also holds the address of a certain byte. That's the difference between
  //  void* and pointer of any other specific types. In short, void* cannot do subscripting
  //  operation (x[i]) or arithmetic operation (x+1), because the compiler cannot know the size
  //  of 1 object void* points to.
  //  To enable such arithmetic operations, we can cast void* to some specific type. For example,
  //  if we want to forward the address by 1 byte, we can do char* x1{static_cast<char*>(x)};
  
}
template<typename T>
void My_Allocator<T>::destroy(T* mem_ptr){
  mem_ptr->~T();		// explicit call of destructor
}
template<typename T>
void My_Allocator<T>::deallocate(T* front_ptr, int element_num){
  free(front_ptr);
  // I think element_num is not used.  free() releases a block of memory kept in one call
  // of malloc(). If I remember correctly, when we do malloc(), the allocated memory contains
  // its byte size in the memory before its front element. That's how a block of memory allocated
  // by malloc() remembers its byte size.
  // <- I checked a StackOverflow post, and the best answer says this is true. But since 
  //    std::allocator<T> has several different implementations, there might be some that
  //    requires the number of elements. But at least when implemeting with malloc() and free(),
  //    the element number is not used.
  // https://stackoverflow.com/questions/38771551/why-does-stdallocatordeallocate-require-a-size

  // What happens if front_ptr has some address other than the pointer returned by malloc()?
  // <- Error "malloc: *** error for object 0x600003330a58: pointer being freed was not allocated"
  //    is generated. I understand this, because the system must find the memory block size,
  //    which is supposed to be stored at a previous (?) address of the front address. So the
  //    system looked at the previous address, but couldn't find appropriate information.
}

// ==============================================================================================

int main()
  try{
  
    My_Allocator<Int> Int_alloc;
    Int* Ip{Int_alloc.allocate(10)};
    for(int i=0; i<10; ++i) Int_alloc.construct(&Ip[i], 1000);
    // check if they are really initialized with .construct()
    for(int i=0; i<10; ++i) cout << Ip[i] << ' ';
    cout << endl;
    for(int i=0; i<10; ++i) Int_alloc.destroy(&Ip[i]);
    Int_alloc.deallocate(Ip, 10);

    // Vector
    My_Allocator<Vector<int>> Vec_alloc;
    Vector<int>* Vp{Vec_alloc.allocate(10)};
    for(int i=0; i<10; ++i) Vec_alloc.construct(&Vp[i], Vector<int>{1,2,3,4,5});
    // check if they are really initialized with .construct()
    for(int i=0; i<10; ++i){
      for(int j=0; j<5; ++j) cout << Vp[i][j] << ' ';
      cout << endl;
    }
    for(int i=0; i<10; ++i) Vec_alloc.destroy(&Vp[i]);
    Vec_alloc.deallocate(Vp, 10);

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
