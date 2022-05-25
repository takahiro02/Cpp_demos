
// copy from ch19/main.cpp

#include "./std_lib_facilities.h"

#include "Hunt_the_Wumpus.h"
using namespace Hunt_the_Wumpus;

int main()
try{
  
  HW_game_window hww{Point{100,100}};
  return gui_main();
  
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
