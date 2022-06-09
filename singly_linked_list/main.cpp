
#include "singly_linked_list.h"
#include<list>

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
  // test insert() (sList<>::insert(p) insers a new element AFTER p, unlike List<>::insert())
    sList<int> slst;
    cout << "### test insert()\n";
    slst.insert(slst.begin(), 1);	// 1 <-> end()
    slst.insert(slst.begin(), 2);	// 1 <-> 2 <-> end()
    auto p = slst.begin();	        // ^-p
    ++p;			        //       ^-p
    p= slst.insert(p, 3);	        // 1 <-> 2 <-> 3 <-> end()
                                        //             ^-p
    slst.insert(p, 4);		        // 1 <-> 2 <-> 3 <-> 4 <-> end()
                                        //             ^-p
                                        // p has not been updated
    print_container(slst);

    // test find_previous()
    cout << "### test find_previous()\n";
    auto q = slst.find_previous(p);
    cout << "*q= " << *q << endl;       //        ^-q 
    
    // test erase()
    cout << "### test erase()\n";
    p = slst.erase(p);		// 1 <-> 2 <-> 4 <-> end()
                                //             ^-p
    p = slst.erase(p);		// 1 <-> 2 <-> end()
                                //             ^-p
    slst.erase(slst.begin());	// 2 <-> end()
                                //       ^-p
    print_container(slst);

    // test push_back()
    cout << "### test push_back()\n";
    slst.push_back(2);		// 2 <-> 2 <-> end()
                                //             ^-p
    slst.push_back(10);		// 2 <-> 2 <-> 10 <-> end()
                                //                    ^-p
    print_container(slst);
    
    // erase all elements
    cout << "### erase all elements and  push_back(10)\n";
    p = slst.erase(slst.begin());	// 2 <-> 10 <-> end()
                                        // ^-p
    p = slst.erase(slst.begin());	// 10 <-> end()
                                        // ^-p
    p = slst.erase(slst.begin());	// end()
                                        // ^-p
    slst.push_back(10);		// in the condition where no element exists, use push_back()
                                // 10 <-> end()
                                //        ^-p
    print_container(slst);

    // test push_front()
    cout << "### test push_front()\n";
    slst.push_front(1);
    slst.push_front(2);
    slst.push_front(3);
    slst.push_front(4);
    print_container(slst);
    // 4 3 2 1 10 

    // test pop_front()
    cout << "### test pop_front()\n";
    slst.pop_front();
    print_container(slst);
    slst.pop_front();
    print_container(slst);
    slst.pop_front();
    print_container(slst);
    slst.pop_front();
    slst.pop_front();
    print_container(slst);
    // slst.pop_front(); // error due to no element
      
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

    // test if sList<Elem>::iterator works in a circulation way
    // currently slst has no elements
    cout << "### test if sList<Elem>::iterator works in a circulation way \n";
    slst.push_back(1);
    slst.push_back(2);
    slst.push_back(3);
    slst.push_back(4);
    print_container(slst);	// 1 2 3 4
    p = slst.end(); ++p;
    cout << "p = slst.end(); ++p; *p= " << *p << endl; // 1 <- ok

    list<int> stl_lst2{1,2,3,4,5};
    print_container(stl_lst2);

    // check initializer_list<T> constructor
    cout << "### check initializer_list<Elem> constructor\n";
    sList<int> slst2{10,20,30,40,50};
    print_container(slst2);

    // check copy constructor
    cout << "### check copy constructor\n";
    sList<int> slst3(slst2);
    cout << "slst3 = ";
    print_container(slst3);
    // check if the elements are different things
    cout << "### slst2.begin().curr= " << slst2.begin().curr << ", slst3.begin().curr" <<
      slst3.begin().curr << endl;

    // to differentiate slst2 from slst3, add 1 to each element
    auto k1 = slst3.begin();
    for(; k1!=slst3.end(); ++k1)
      *k1 = *k1+1;
    
    cout << "slst = ";
    print_container(slst);
    cout << "slst2 = ";
    print_container(slst2);
    cout << "slst3 = ";
    print_container(slst3);
    cout << "slst.size()= " << slst.size() << ", " << "slst2.size()= " << slst2.size() << ", "
	 << "slst3.size()= " << slst3.size() << endl;

    return 0;
  }
  catch(exception& e){
    cerr << e.what() << endl;
    return 1;
  }
  catch(...){
    cerr << "Unknown exception is caught\n";
    return 1;
  }
