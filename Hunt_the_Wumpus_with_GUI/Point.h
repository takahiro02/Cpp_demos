
//
// This is a GUI support code to the chapters 12-16 of the book
// "Programming -- Principles and Practice Using C++" by Bjarne Stroustrup
//

#ifndef POINT_GUARD
#define POINT_GUARD

#include"std_lib_facilities.h"

namespace Graph_lib {
//------------------------------------------------------------------------------

struct Point {
    int x, y;
};

//------------------------------------------------------------------------------

inline bool operator==(Point a, Point b) { return a.x==b.x && a.y==b.y; } 

//------------------------------------------------------------------------------

inline bool operator!=(Point a, Point b) { return !(a==b); }

//------------------------------------------------------------------------------

  // for debug
  inline ostream& operator<<(ostream& os, const Point& p){
    os << "(" << p.x << ", " << p.y << ")";
    return os;
  }
  /*
  make the function inline if it is defined in a header file
  otherwise a linker error happens
  see: https://stackoverflow.com/questions/2219243/odd-duplicate-symbols-error
  inline is similar to macros in C: it replaces an inline function call with the
  body of the function, instead of referring to the function's definition
  located somewhere in the executable binary file. Since inline functions
  replace all of their function calls with their function body in the
  compilation, the executable binary files get bigger than otherwise.
  One difference from C's macro is that inline functions can check the argument
  types. Another difference is that inlining in C++ is managed by C++ compiler,
  whereas C (and C++)'s macros are managed by preprocessor.
  see: https://www.geeksforgeeks.org/inline-functions-cpp/
  Also, as written in the above page, if a function is defined inside a class
  declaration, that function is implicitly translated as an inline function
  Here is some Q&A of inline: https://isocpp.org/wiki/faq/inline-functions
  
  I also got a linker error of multiple definition if I don't attach "inline" 
  keyword to this operator overload.
  This is because the compilation of each cpp file is done separately (for main.o
  , Graph.o, GUI.o, Simple_window.o, Window.o), and since include guard works only
  for one compilation unit (for compiling each object file), it cannot guard
  if a function definition is included in a header file and that header file is
  included in several .cpp files, and they are compiled separately, as in my case:
g++  -c main.cpp -o main.o -std=c++14
g++  -c Graph.cpp -o Graph.o -std=c++14
g++  -c GUI.cpp -o GUI.o -std=c++14
g++  -c Simple_window.cpp -o Simple_window.o -std=c++14
g++  -c Window.cpp -o Window.o -std=c++14
g++  -o main main.o Graph.o GUI.o Simple_window.o Window.o -std=c++14 `fltk-config --ldflags --use-images`

  (https://stackoverflow.com/questions/34997795/)
  In such a case, when linking these separate object files, a linker error of 
  multiple definition of the same function happens.
  To prevent that, avoid defining functions inside a header file, just declare
  them in a header file and define them in a separate cpp file. Or if the function 
  definition is really small (hence it is likely to get the advantage of inlining it from 
  removing function overheads), by attaching "inline" keyword, compilers would 
  allow multiple definition of inline functions, and would compile successfully.
  (https://cboard.cprogramming.com/cplusplus-programming/116825-inline-function-header-multiple-definition.html)
  But it is programmers' responsibility to ensure that each of the multiple inline
  function definitions are the same definition (has the same function body)
  */
  
}
#endif // POINT_GUARD

