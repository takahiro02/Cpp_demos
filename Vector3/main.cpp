
// copy from ch19/main.cpp

#include "./std_lib_facilities.h"

#include "vector3.h"

template<typename T>
void print_vec(const Vector3<T>& v){
  for(int i=0; i<v.size(); ++i)
    cout << v[i] << ' ';
  cout << endl;
}

int main()
try{
  // test Vector3
  Vector3<int> int_v3(3, 10);
  print_vec(int_v3);
  cout << "### int_v3.capacity()= " << int_v3.capacity() << ", int_v3.size()= " << int_v3.size() << endl;
  int_v3.reserve(20);
  cout << "### int_v3.capacity()= " << int_v3.capacity() << ", int_v3.size()= " << int_v3.size() << endl;
  int_v3.push_back(4), int_v3.push_back(5), int_v3.push_back(6);
  print_vec(int_v3);
  cout << "### int_v3.capacity()= " << int_v3.capacity() << ", int_v3.size()= " << int_v3.size() << endl;
  int_v3.resize(10);
  cout << "### after resize()\n";
  print_vec(int_v3);
  cout << "### int_v3.capacity()= " << int_v3.capacity() << ", int_v3.size()= " << int_v3.size() << endl;
  Vector3<int> int_v3_2{int_v3};
  cout << "### int_v3_2.capacity()= " << int_v3_2.capacity() << ", int_v3_2.size()= " << int_v3_2.size() << endl;
  print_vec(int_v3_2);

  cout << "### sizeof(int_v3)= " << sizeof(int_v3) << endl;
  Vector3<Vector3<Vector3<int>>> vecvecvec_int;
  cout << "### sizeof(vecvecvec_int)= " << sizeof(vecvecvec_int) << endl;

  
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
