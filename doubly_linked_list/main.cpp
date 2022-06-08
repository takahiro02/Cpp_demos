
#include<iostream>
#include<list>
#include "doubly_linked_list.h"

using namespace std;

// for debug
// print_container() cannot use with arrays, since they don't have .begin() or .end() members
template<typename C>		// concept predicate (p685)
//requires Container<C>()
// I couldn't use Container concept, it seems I have to define my own concept for checking
// STL container.
// https://stackoverflow.com/questions/9407367/
void print_container(const C& c){
  for(const auto e : c)
    cout << e << " ";
  cout << endl;
}

int main()
  try{
    List<int> lst;
    cout << "### test insert()\n";
    lst.insert(lst.begin(), 1);	// 1 <-> end()
    lst.insert(lst.begin(), 2);	// 2 <-> 1 <-> end()
    auto p = lst.begin();	// ^-p
    ++p;			//       ^-p
    p= lst.insert(p, 3);	// 2 <-> 3 <-> 1 <-> end()
                                //       ^-p
    lst.insert(p, 4);		// 2 <-> 4 <-> 3 <-> 1 <-> end()
                                //             ^-p
    print_container(lst);
    
    // test erase()
    cout << "### test erase()\n";
    p = lst.erase(p);		// 2 <-> 4 <-> 1 <-> end()
                                //             ^-p
    p = lst.erase(p);		// 2 <-> 4 <-> end()
                                //             ^-p
    lst.erase(lst.begin());	// 4 <-> end()
                                //       ^-p
    print_container(lst);

    // test push_back()
    cout << "### test push_back()\n";
    lst.push_back(2);		// 4 <-> 2 <-> end()
                                //             ^-p
    lst.push_back(10);		// 4 <-> 2 <-> 10 <-> end()
                                //                    ^-p
    print_container(lst);
    
    // erase all elements
    p = lst.erase(lst.begin());	// 2 <-> 10 <-> end()
                                // ^-p
    p = lst.erase(lst.begin());	// 10 <-> end()
                                // ^-p
    p = lst.erase(lst.begin());	// end()
                                // ^-p
    lst.push_back(10);		// in the condition where no element exists, use push_back()
                                // 10 <-> end()
                                //        ^-p
    print_container(lst);

    // unsigned long l{1};
    // cout << "### unsigned long l = " << l << endl;
    // --l;
    // cout << "### unsigned long l = " << l << endl;
    // --l;
    // cout << "### unsigned long l = " << l << endl;

    // test push_front()
    cout << "### test push_front()\n";
    lst.push_front(1);
    lst.push_front(2);
    lst.push_front(3);
    lst.push_front(4);
    print_container(lst);
    // 4 3 2 1 10 

    // test pop_front()
    cout << "### test pop_front()\n";
    lst.pop_front();
    print_container(lst);
    lst.pop_front();
    print_container(lst);
    lst.pop_front();
    print_container(lst);
    lst.pop_front();
    lst.pop_front();
    print_container(lst);
    // lst.pop_front(); // error due to no element

    // test pop_back()
    cout << "### test pop_back()\n";
    lst.push_front(1);
    lst.push_front(2);
    lst.push_front(3);
    lst.push_front(4);
    print_container(lst);
    lst.pop_back();
    print_container(lst);
    lst.pop_back();
    print_container(lst);
    lst.pop_back();
    print_container(lst);
    lst.pop_back();
    //lst.pop_back();
    print_container(lst);
    // lst.pop_back(); // error due to no element 
      
    // ex 13
    // check if -- can be used to end() in STL's list
    cout << "### test STL's list<Elem>::end()\n";
    list<int> stl_lst{1,2,3};
    auto p1 = stl_lst.end();
    --p1;
    print_container(stl_lst);
    cout << "### --stl_lst.end()= " << *p1 << endl;
    ++p1; ++p1;
    cout << "### ++stl_lst.end()= " << *p1 << endl;
    // it gave 3, so operator--() can be correctly used even for end() element, for STL's
    // list<Elem>
    Link<int>* li{0};
    if(li==0)
      cout << "### Link<int>* li = 0 is valid\n";

    // test if List<Elem>::iterator works in a circulation way
    cout << "### test if List<Elem>::iterator works in a circulation way \n";
    lst.push_back(1);
    lst.push_back(2);
    lst.push_back(3);
    lst.push_back(4);
    print_container(lst);
    p = lst.begin(); --p;
    cout << "*p= " << *p << endl;
    p = lst.end(); ++p;
    cout << "*p= " << *p << endl;

    list<int> stl_lst2{1,2,3,4,5};
    print_container(stl_lst2);

    // check initializer_list<T> constructor
    cout << "### check initializer_list<Elem> constructor\n";
    List<int> lst2{1,2,3,4,5};
    print_container(lst2);

    // check copy constructor
    cout << "### check copy constructor\n";
    List<int> lst3{lst2};
    cout << "lst3 = ";
    print_container(lst3);
    // check if the elements are different things
    cout << "### lst2.begin().curr= " << lst2.begin().curr << ", lst3.begin().curr" <<
      lst3.begin().curr << endl;

    // check copy assignment operator
    cout << "### check copy assignment operator\n";
    List<int> lst4{1,2};
    lst4 = lst3;
    cout << "lst4 = ";
    print_container(lst4);

  }
  catch(exception& e){
    cerr << e.what() << endl;
    return 1;
  }
  catch(...){
    cerr << "Unknown error\n";
    return 1;
  }
