
// Ex 16 ch19

// the header file is not included at the top here. The reason is specific to the template
// definition. See my comments in the end of vector3.h

// copy operator (contents are based on Vector's copy operator in main.cpp)
// the "this->" s in this function are not necessary, because the pointed members are not from
// the base class
template<typename T>
Vector3<T>& Vector3<T>::operator=(const Vector3& a){
  if(this == &a) return *this;  // self-assignment, no work needed

  // It's possible that a user make an empty Vector3<T>, and try to call this function.
  // In that case, vec_data-> causes segmentation fault, since vec_data is still nullptr.
  // So we first need to create the data
  if(vec_data == nullptr) vec_data = new vector_data<T>; // empty vector_data

  
  if(a.vec_data->sz<=this->vec_data->space){	// enough space, no need for new allocation
    for(int i=0; i<a.vec_data->sz; ++i)
      this->vec_data->elem[i] = a.vec_data->elem[i]; // copy elements
    this->vec_data->sz = a.vec_data->sz;
    return *this;
  }

  // case where this object's allocated memory is not enough to copy all elements of a, so
  // allocation of a's size is done as well
  unique_ptr<T> p{this->vec_data->alloc.allocate(a.vec_data->sz)};

  uninitialized_copy(a.vec_data->elem, a.vec_data->elem[a.vec_data->sz], p);
  // Instead of using for-loop and alloc.construct(), I use uninitialized_copy(), because if
  // in the middle of the initialization, some exception happens, it destroys already constructed
  // elements.
  // I don't need try{}catch(){} block here, unlike vector_data's constructors, because
  // unique_ptr<T> is RAII, meaning when p is destroyed, its pointed objects are deleted.
  // I don't know if the objects are properly deleted, because they are aquired by
  // alloc.allocate(), instead of new. See the "Notice" comment below the first constructor
  // of vector_data for the reason of my concern.
  
  // Notice: since construction on p is based on a, the iteration size is a.sz, but since
  //         destrution is based on the previous allocated memory elem, its iteration size
  //         is sz, and deallocation is done with this->space
  for(int i=0; i<this->vec_data->sz; ++i)
    this->vec_data->alloc.destruct(&this->vec_data->elem[i]);
  this->vec_data->alloc.deallocate(this->vec_data->elem, this->vec_data->space);

  this->vec_data->space = this->vec_data->sz = a.vec_data->sz;	// means space = (sz = a.sz);
  this->vec_data->elem = p.release();
  // from p703, to prevent the allocated memory from being dealocated
  // p.release() sets p to nullptr
  
  return *this;
}

// move assignment operator
// I think move assignment is used when the right hand side of = is a function returning its
// local variable vector<T>, which is about to be destroyed, and the left hand side was already
// constructed. For example, assume fill(cin) returns its locally filled Vector3<T>, then, 
// Vector3<int> vec_int(fill(cin)); // vec_int is now created, so move constructor is used
// Vector3<int> vec_int2;
// vec_int2 = fill(cin); // vec_int2 was already constructed, so move assignment is used
template<typename T>
Vector3<T>& Vector3<T>::operator=(Vector3&& a){
  // main.cpp's Vector's move assignment tries not to shrink the original space, so it does
  // copying each element if a's elem size is smaller than this object. But in this Vector3's
  // move assignment, I decided not to care shrinking the original allocated space, and decided
  // to directly use the a's allocated memory

  // It's possible that a user make an empty Vector3<T>, and try to call this function.
  // In that case, vec_data-> causes segmentation fault, since vec_data is still nullptr.
  // So we first need to create the data
  if(vec_data == nullptr) vec_data = new vector_data<T>; // empty vector_data
  
  // before moving a's memory, destroy and deallocate the originally allocated memory
  for(int i=0; i<this->vec_data->sz; ++i)
    this->vec_data->alloc.destroy(&this->vec_data->elem[i]);
  this->vec_data->alloc.deallocate(this->vec_data->elem, this->vec_data->space);
  
  vec_data = a.vec_data;
  a.vec_data = nullptr;
  // Since a.vec_data is set to nullptr, a's allocated memory is no longer destroyed or
  // deallocated by a's destructor (see Vector3's destructor)
}

// version of reserve() that uses unique_ptr
// based on Vector<T,A>::reserve() in main.cpp
template<typename T>
void Vector3<T>::reserve(int newalloc){
  // It's possible that a user make an empty Vector3<T>, and try to call this function.
  // In that case, vec_data-> causes segmentation fault, since vec_data is still nullptr.
  // So we first need to create the data
  if(vec_data == nullptr) vec_data = new vector_data<T>; // empty vector_data
  
  if(newalloc <= this->vec_data->space)
    return;			// never decrease allocation

  unique_ptr<T[]> p{this->vec_data->alloc.allocate(newalloc)};
  // unique_ptr<T> is basically for only 1 object. It is not for an array of objects. That's
  // why it doesn't provide any subscript [] operator, or + operator (usually pointer+i gives
  // the address i objects forward from pointer). So, it deallocates the memory with delete,
  // usually. If we want to use it for an array, we can say unique_ptr<T[]> (notice the square
  // brackes). That way, the deallocation happens with delete[]. And if we use T[] instead of
  // T in unique_ptr<>, then the subscript operator becomes available (+ operator is till
  // not available)
  // https://stackoverflow.com/questions/8940931/

  int i{0};
  // to keep the index before which the elements' construction was successful, define i outside
  // of try{}catch(){} block
  try{
    for(i=0; i<this->vec_data->sz; ++i)
      this->vec_data->alloc.construct(&p[i], this->vec_data->elem[i]);
  }
  catch(...){
    for(int j=0; i<i; ++j)
      this->vec_data->alloc.destroy(&p[i]);
    // destroy already constructed elements
    // I don't have to deallocate the elements pointed by p, because unique_ptr is RAII, so
    // when p is destroyed, its pointed elements are deallocated automatically with delete[].
    // But to my knowledge, if we allocate memory with allocator<T>::allocate(), using delete[]
    // causes error (see my comments in the "Notice" part below the first constructor of
    // vector_data), so I don't know if the deallocation succeeds.
  }
  //uninitialized_copy(this->vec_data->elem, this->vec_data->elem[this->vec_data->sz], p);
  // Instead of using for-loop and alloc.construct(), I use uninitialized_copy(), because if
  // in the middle of the initialization, some exception happens, it destroys already constructed
  // elements.
  // I don't need try{}catch(){} block here, unlike vector_data's constructors, because
  // unique_ptr<T> is RAII, meaning when p is destroyed, its pointed objects are deleted.
  // I don't know if the objects are properly deleted, because they are aquired by
  // alloc.allocate(), instead of new. See the "Notice" comment below the first constructor
  // of vector_data for the reason of my concern.

  
  // Since unique_ptr<T> doesn't provide subscripting [] or forwarding + operator, we have
  // to get the address i objects forward with p.get()+i, instead of &p[i] or p+i.
  // But if we use unique_ptr<T[]>, then the subscripting operator becomes available.
  for(int i=0; i<this->vec_data->sz; ++i)
    this->vec_data->alloc.destroy(&this->vec_data->elem[i]);	   // destroy
  this->vec_data->alloc.deallocate(this->vec_data->elem, this->vec_data->space);
  // deallocate old space
  
  this->vec_data->elem = p.release();
  this->vec_data->space = newalloc;
  // this->vec_data->sz stays the same
}

// version of reserve() that uses vector_data
// based on Vector<T,A>::reserve2() in main.cpp
template<typename T>
void Vector3<T>::reserve2(int newalloc){
  // It's possible that a user make an empty Vector3<T>, and try to call this function.
  // In that case, vec_data-> causes segmentation fault, since vec_data is still nullptr.
  // So we first need to create the data
  if(vec_data == nullptr) vec_data = new vector_data<T>; // empty vector_data
  
  if(newalloc <= this->vec_data->space)
    return;			// never decrease allocation
  
  vector_data<T> b;		// use default constructor prepared for this time
  b.space = newalloc;
  b.sz = this->vec_data->sz;
  b.elem = b.alloc.allocate(newalloc);

  // copy the existing elements of this object to b
  try{
    uninitialized_copy(this->vec_data->elem, &(this->vec_data->elem[this->vec_data->sz]), b.elem);
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

  T* tp{this->vec_data->elem};
  int tspace{this->vec_data->space}, tsz{this->vec_data->sz};
  
  this->vec_data->elem = b.elem;
  this->vec_data->space = b.space;
  // this->vec_data->sz stays the same

  // to properly destroy and deallocate the old memory by b's destructor,
  // assign the old information to b
  b.elem = tp, b.space = tspace, b.sz = tsz;
}

// The difference of this function from Vector::reserve() is that this function initializes
// the reserved, but free space with val (2nd argument), and changes sz to the newsize, 
// whereas reserve() doesn't initialize the free, reserved elements, or change only variable
// "space", not sz
template<typename T>
void Vector3<T>::resize(int newsize, T val){ // default value (p690)
  reserve(newsize);

  // There are 2 cases, one is when newsize => sz, and the other is newsize < sz
  // This 1st for-loop is for the 1st case. Initialize the allocated elements that has not
  // been initialized in reserve()
  for(int i=this->vec_data->sz; i<newsize; ++i)
    this->vec_data->alloc.construct(&this->vec_data->elem[i], val);

  // This 2nd for-loop is for the latter case, which is only for when newsize < sz. 
  // In that case, the previously stuffed elements are in elem[newsize] ~ elem[sz-1], 
  // so we need to call a destructor for these elements. See my note for the diagram.
  for(int i=newsize; i<this->vec_data->sz; ++i)
    this->vec_data->alloc.destroy(&this->vec_data->elem[i]);
  
  this->vec_data->sz = newsize;
}

// based on main.cpp's Vector<T,A>::push_back()
template<typename T>
void Vector3<T>::push_back(const T& val){
  // It's possible that a user make an empty Vector3<T>, and try to call this function.
  // In that case, vec_data-> causes segmentation fault, since vec_data is still nullptr.
  // So we first need to create the data
  if(vec_data == nullptr) vec_data = new vector_data<T>; // empty vector_data

  if(this->vec_data->space==0)
    reserve(8);
  else if(this->vec_data->sz==this->vec_data->space)
    reserve(2*this->vec_data->space);

  this->vec_data->alloc.construct(&this->vec_data->elem[this->vec_data->sz],val);
  
  ++this->vec_data->sz;
  // sz points to 1 element beyond the last element
}

