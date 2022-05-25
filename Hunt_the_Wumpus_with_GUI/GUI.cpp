
//
// This is a GUI support code to the chapters 12-16 of the book
// "Programming -- Principles and Practice Using C++" by Bjarne Stroustrup
//

#include <FL/Fl.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Output.H>
#include "GUI.h"
#include <string>

#include <FL/Fl_Multiline_Output.H> // for multiple-line output box in HW_game_window class
// of Exercise 14, ch19
// from https://stackoverflow.com/questions/18758462/stroustrup-fltk-out-box-newline
// Mysteriously, when I include this header file in GUI.h, the Fl_Multiline_Output class was not
// recognized by the compiler. Maybe this is because this class is a child class of Fl_output
// class, so including it before including Fl_Output.H doesn't allow deriving the
// Fl_Multiline_Output class properly?

namespace Graph_lib {

//------------------------------------------------------------------------------

void Button::attach(Window& win)
{
    pw = new Fl_Button(loc.x, loc.y, width, height, label.c_str());
    pw->callback(reinterpret_cast<Fl_Callback*>(do_it), &win); // pass the window
    // This callback function is from Fl_Button object, but according to the FLTK doc
    // website https://www.fltk.org/doc-1.3/classFl__Button.html, Fl_Button class
    // doesn't have a member callback, but its base class Fl_Widget has. Thus I think
    // this callback is from Fl_Widget class.
    // The 1st argument do_it is a function pointer having the address of a
    // function whose return type is void, and takes 2 Addrress (== void*) type arguments
    // Since the types of the 2 arguments is Fl_Widget::callback(Fl_Callback *, void*)
    // So Fl_Widget::callback(function to be called when this Fl_Button is pressed,
    //                        any pointer to be passed to the 2nd argument of the function
    //                         to be called)

    //pw->callback(reinterpret_cast<Fl_Callback*>(do_it), this);
    // This works too. In this case, the pointer (address) to this Button object itself
    // is passed to the 2nd argument of the function pointed by do_it
    // But if I do this, Buttons attached to a Window class cannot have its distinct
    // meaning to the Window, since remember, in a user-defined callback function like
    // Lines_window::cb_red(), it calls a member function defined in that Window class.
    // In that way, we can make a Button have different actions for different Window types.
    // But if we set the 2nd argument to "this" like above, the only member functions
    // that can be called from a user-defined callback function is member functions
    // of Button class. Thus we cannot make Buttons have different meaning for different
    // Window type.
    
    own = &win;
}

//------------------------------------------------------------------------------

int In_box::get_int()
{
    Fl_Input& pi = reference_to<Fl_Input>(pw);
    // return atoi(pi.value());
    const char* p = pi.value();
    if (!isdigit(p[0])) return -999999;
    return atoi(p);
}

//------------------------------------------------------------------------------

string In_box::get_string()
{
	Fl_Input *p = static_cast<Fl_Input *>(pw);

	return string{p->value()};
}

//------------------------------------------------------------------------------

void In_box::attach(Window& win)
{
    pw = new Fl_Input(loc.x, loc.y, width, height, label.c_str());
    own = &win;
}

//------------------------------------------------------------------------------

void Out_box::put(const string& s)
{
  //reference_to<Fl_Output>(pw).value(s.c_str());
  // To allow multi-line output box, I modified it to the following
  // This is for Exercise 14, ch19
  // from https://stackoverflow.com/questions/18758462/stroustrup-fltk-out-box-newline
  reference_to<Fl_Multiline_Output>(pw).value(s.c_str());
}

//------------------------------------------------------------------------------

void Out_box::attach(Window& win)
{
  //pw = new Fl_Output(loc.x, loc.y, width, height, label.c_str());
  // To allow multi-line output box, I modified it to the following
  // This is for Exercise 14, ch19
  // from https://stackoverflow.com/questions/18758462/stroustrup-fltk-out-box-newline
  pw = new Fl_Multiline_Output(loc.x, loc.y, width, height, label.c_str());
    own = &win;
}

//------------------------------------------------------------------------------

Menu::Menu(Point xy, int w, int h, Kind kk, const string& label)
            : Widget(xy,w,h,label,0), k(kk), offset(0)
        {}

//------------------------------------------------------------------------------

void Menu::hide()                 // hide all buttons
{
    for (unsigned int i = 0; i<selection.size(); ++i) 
	selection[i].hide(); 
}

//------------------------------------------------------------------------------

void Menu::move(int dx, int dy)   // move all buttons
{
    for (unsigned int i = 0; i<selection.size(); ++i) 
	selection[i].move(dx,dy);
}

//------------------------------------------------------------------------------

void Menu::attach(Window& win)    // attach all buttons
{
    for (int i=0; i<selection.size(); ++i) win.attach(selection[i]);
    own = &win;
}
	  
//------------------------------------------------------------------------------

int Menu::attach(Button& b)
{
    b.width = width;
    b.height = height;

    switch(k) {
    case horizontal:
        b.loc = Point{loc.x+offset,loc.y};
        offset+=b.width;
        break;
    case vertical:
        b.loc = Point{loc.x,loc.y+offset};
        offset+=b.height;
        break;
    }
    selection.push_back(b); // b is NOT OWNED: pass by reference
    return int(selection.size()-1);
}

//------------------------------------------------------------------------------

int Menu::attach(Button* p)
{
    Button& b = *p;
    b.width = width;
    b.height = height;

    switch(k) {
    case horizontal:
        b.loc = Point{loc.x+offset,loc.y};
        offset+=b.width;
        break;
    case vertical:
        b.loc = Point{loc.x,loc.y+offset};
        offset+=b.height;
        break;
    }
    selection.push_back(&b); // b is OWNED: pass by pointer
    return int(selection.size()-1);
}

//------------------------------------------------------------------------------

  /* // for Drill 2
  Lines_window::Lines_window(Point xy, int w, int h, const string& title)
    : Window{xy, w, h, title},
      next_button{Point{x_max()-150,0}, 70, 20, "Next point",
		  [](Address, Address pw){reference_to<Lines_window>(pw).next();}},
      quit_button{Point{x_max()-70,0}, 70, 20, "Quit",
		  [](Address, Address pw){reference_to<Lines_window>(pw).quit();}},
      next_x{Point{x_max()-310,0}, 50, 20, "next x:"},
      next_y{Point{x_max()-210,0}, 50, 20, "next y:"},
      xy_out{Point{100,0}, 100, 20, "current (x,y):"}
  {
    attach(next_button);
    attach(quit_button);
    attach(next_x);
    attach(next_y);
    attach(xy_out);
    attach(lines);
  }
  */
  
  // for Drill 3
  Lines_window::Lines_window(Point xy, int w, int h, const string& title)
    : Window{xy, w, h, title},
      next_button{Point{x_max()-150,0}, 70, 20, "Next point", cb_next},
      quit_button{Point{x_max()-70,0}, 70, 20, "Quit", cb_quit},
      next_x{Point{x_max()-310,0}, 50, 20, "next x:"},
      next_y{Point{x_max()-210,0}, 50, 20, "next y:"},
      xy_out{Point{100,0}, 100, 20, "current (x,y):"},
      color_menu{Point{x_max()-70,30}, 70, 20, Menu::vertical, "color"},
      menu_button{Point{x_max()-80,30}, 80, 20, "color menu", cb_menu},
      // Drill 4
      style_menu{Point{x_max()-70,100}, 70, 20, Menu::vertical, "style"},
      style_menu_button{Point{x_max()-80,100}, 80, 20, "style menu", cb_style_menu}
  {
    attach(next_button);
    attach(quit_button);
    attach(next_x);
    attach(next_y);
    attach(xy_out);
    xy_out.put("no point");
    color_menu.attach(new Button{Point{0,0}, 0, 0, "red", cb_red});
    color_menu.attach(new Button{Point{0,0}, 0, 0, "blue", cb_blue});
    color_menu.attach(new Button{Point{0,0}, 0, 0, "black", cb_black});
    attach(color_menu);
    color_menu.hide();
    attach(menu_button);
    attach(lines);

    // Drill 4
    style_menu.attach(new Button{Point{0,0}, 0, 0, "dash", cb_dash});
    style_menu.attach(new Button{Point{0,0}, 0, 0, "solid", cb_solid});
    style_menu.attach(new Button{Point{0,0}, 0, 0, "dot", cb_dot});
    // Notice: the location of Button can be anywhere, because when attached to a
    // Menu class, its location is adjusted in int Menu::attach(Button& b)
    attach(style_menu);
    style_menu.hide();
    attach(style_menu_button);
  }
   // for Drill 3

  // from p567
  void Lines_window::next(){
    int x = next_x.get_int();
    int y = next_y.get_int();

    lines.add(Point{x,y});

    // update current position readout:
    ostringstream ss;
    ss << '(' << x << ',' << y << ')';
    xy_out.put(ss.str());

    redraw();
  }

  // Ex 1, 2 of chapter 16
My_window::My_window(Point tl, int w, int h, const string& title)
  : Window{tl, w, h, title},
    next{Point{x_max()-150, 0}, 70, 20, "Next",
	 [](Address, Address pw){reference_to<My_window>(pw).next_pressed();}},
    quit{Point{x_max()-70, 0}, 70, 20, "Quit",
	 [](Address, Address pw){reference_to<My_window>(pw).quit_pressed();}},
    checkboard1{Point{100,100}, 100,100, Menu::vertical, "Checkboard menu 1"},
    checkboard2{Point{200,100}, 100,100, Menu::vertical, "Checkboard menu 2"},
    checkboard3{Point{300,100}, 100,100, Menu::vertical, "Checkboard menu 3"},
    checkboard4{Point{400,100}, 100,100, Menu::vertical, "Checkboard menu 4"}
{
  attach(next);
  attach(quit);

  // add 4 Buttons to each Menu
  /*
// this way of using the same callback function to all the checkboard Buttons doesn't work
// if the Window object having the Buttons is passed to Fl_Widget::callback() in 
// Button::attach(). If I pass the Button object itself to Fl_Widget::callback() there,
// this way could work, but in that case, I need to define all the possible actions
// of Button (not only for My_widnow, but for any Window class using Button) inside
// Button class
  for(int i=1; i<5; ++i){
    for(int j=1; j<5; ++j){
      switch(i){
      case 1:
	checkboard1.attach(new Button{Point{0,0}, 100, 100, "", cb_checkboard});
	break;
      case 2:
	checkboard2.attach(new Button{Point{0,0}, 100, 100, "", cb_checkboard});
	break;
      case 3:
	checkboard3.attach(new Button{Point{0,0}, 100, 100, "", cb_checkboard});
	break;
      case 4:
	checkboard4.attach(new Button{Point{0,0}, 100, 100, "", cb_checkboard});
	break;
      }
    }
  }
  */

  checkboard1.attach(new Button{Point{0,0}, 100, 100, "", cb_m1b1});
  checkboard1.attach(new Button{Point{0,0}, 100, 100, "", cb_m1b2});
  checkboard1.attach(new Button{Point{0,0}, 100, 100, "", cb_m1b3});
  checkboard1.attach(new Button{Point{0,0}, 100, 100, "", cb_m1b4});
  checkboard2.attach(new Button{Point{0,0}, 100, 100, "", cb_m2b1});
  checkboard2.attach(new Button{Point{0,0}, 100, 100, "", cb_m2b2});
  checkboard2.attach(new Button{Point{0,0}, 100, 100, "", cb_m2b3});
  checkboard2.attach(new Button{Point{0,0}, 100, 100, "", cb_m2b4});
  checkboard3.attach(new Button{Point{0,0}, 100, 100, "", cb_m3b1});
  checkboard3.attach(new Button{Point{0,0}, 100, 100, "", cb_m3b2});
  checkboard3.attach(new Button{Point{0,0}, 100, 100, "", cb_m3b3});
  checkboard3.attach(new Button{Point{0,0}, 100, 100, "", cb_m3b4});
  checkboard4.attach(new Button{Point{0,0}, 100, 100, "", cb_m4b1});
  checkboard4.attach(new Button{Point{0,0}, 100, 100, "", cb_m4b2});
  checkboard4.attach(new Button{Point{0,0}, 100, 100, "", cb_m4b3});
  checkboard4.attach(new Button{Point{0,0}, 100, 100, "", cb_m4b4});
  
  attach(checkboard1);
  attach(checkboard2);
  attach(checkboard3);
  attach(checkboard4);

}

/*
void My_window::cb_checkboard(Address bp, Address pw){
  // bp is a pointer to the Fl_Button object, not Button, that calls this callback function
  // pw can be any pointer. We can freely set it in the 2nd argument of
  // Fl_Widget::callback()
  // Since bp points to a Fl_Button object, we cannot use any user-defined member functions.
  // So the freedom of using bp is limited, and I think that's why the author of this book,
  // PPP, decided not to use the 1st argument.
  
  reference_to<My_window>(pw).checkboard_pressed();
  // this cast to My_window* is needed, because Address is void*, so compiler cannot
  // know if the object pw is pointing to really has member function checkboard_pressed().
  // So this cast is simply to pass the compilation.
  // If pw is in fact pointing to, say, a Button object, compilation still succeeds,
  // because compiler doesn't know what pw is really pointing to. But it causes
  // a segmentation fault in the run time, or some other error.
  
  //reference_to<Button>(pw).set_color(FL_BLUE);
  // If I pass "this" to the 2nd argument of Fl_Widget::callback() in Button::attach(),
  // this function also works.
  // I decided to pass the Window object to the 2nd argument of Fl_Button::callback() in
  // Button::attach(), for the reason I wrote in the comment in Button::attach() (the
  // 2nd block of the comments)
  
  //reference_to<Fl_Button>(bp).setonly();
}
*/

// clear all the Buttons in the checkboard, and set only the Button specified in the 
// arguments
void My_window::checkboard_push_only_1(int menu_i, int button_i){

  clear_all_cbbuttons();
  
  switch(menu_i){
  case 1:
    checkboard1.selection[button_i-1].set();
    // -1 is due to 1-indexing of button_i
    break;
  case 2:
    checkboard2.selection[button_i-1].set();
    break;
  case 3:
    checkboard3.selection[button_i-1].set();
    break;
  case 4:
    checkboard4.selection[button_i-1].set();
    break;
  }
}

void My_window::clear_all_cbbuttons(){
  for(int i=0; i<4; ++i){
    checkboard1.selection[i].clear();
    checkboard2.selection[i].clear();
    checkboard3.selection[i].clear();
    checkboard4.selection[i].clear();
  }
}


  // Ex 3 of chapter 16
  // version 1 constructor
/*
Image_window::Image_window(Point tl, int w, int h, const string& title)
  : Window{tl, w, h, title},
    quit{Point{x_max()-70, 0}, 70, 20, "Quit",
	 [](Address, Address pw){reference_to<Image_window>(pw).quit_pressed();}},
    image_b{Point{x_max()/2-50, y_max()/2-50}, 100, 100, "",
	    [](Address, Address pw){reference_to<Image_window>(pw).image_pressed();}}
{
  attach(quit);
  attach(image_b);
}
*/

// version 2 constructor (comment this out if you want to use version 1)
Image_window::Image_window(Point tl, int w, int h, const string& title,
			   const string& img_name, Point xy, int iw, int ih)
  : Window{tl, w, h, title},
    quit{Point{x_max()-70, 0}, 70, 20, "Quit",
	 [](Address, Address pw){reference_to<Image_window>(pw).quit_pressed();}},
    image_b{Point{x_max()/2-50, y_max()/2-50}, 100, 100, "",
	    [](Address, Address pw){reference_to<Image_window>(pw).image_pressed();}},
    image{Point{x_max()/2-50, y_max()/2-50}, img_name}
{
  attach(quit);
  attach(image_b);
  
  image.set_mask(xy, iw, ih);
  attach(image);
}

// version 2 image_pressed() (comment this out if you want to use version 1)
void Image_window::image_pressed(){
  int new_x{rand_int(0, x_max()-image_b.width)};
  int new_y{rand_int(0, y_max()-image_b.height)};
  // for Image::move(), I need to know how many pixels it moves relative to the previous
  // Image position. So get the current position of the Button before it moves
  int now_x{image_b.loc.x};
  int now_y{image_b.loc.y};
  image_b.position(new_x, new_y);

  image.move(new_x-now_x, new_y-now_y);
}


// Ex 4
Shape_window::Shape_window(Point tl, int w, int h, const string& title)
  : Window{tl, w, h, title},
    quit{Point{x_max()-70, 0}, 70, 20, "Quit",
	 [](Address, Address pw){reference_to<Shape_window>(pw).quit_pressed();}},
    xi{Point{x_max()-310,0}, 50, 20, "x coord:"},
    yi{Point{x_max()-200,0}, 50, 20, "y coord:"},
    shape_menu{Point{x_max()-90,30}, 90, 20, Menu::vertical, "shape"},
    shape_menu_bt{Point{x_max()-90, 30}, 90, 20, "Shape menu", cb_shapemenu}
{
  attach(quit);
  attach(xi);
  attach(yi);
  attach(shape_menu_bt);
  
  shape_menu.attach(new Button{Point{0,0}, 0, 0, "Circle", cb_circle});
  shape_menu.attach(new Button{Point{0,0}, 0, 0, "Square", cb_square});
  shape_menu.attach(new Button{Point{0,0}, 0, 0, "Equi-triangle", cb_equi_tri});
  shape_menu.attach(new Button{Point{0,0}, 0, 0, "Hexagon", cb_hexagon});
  // we don't have to set the Button's Point, width, and height, since they are
  // later adjusted with the Menu's location, and width and height
  
  attach(shape_menu);
  shape_menu.hide();		// at first, shape_menu is invisible
}

Shape_window::XY Shape_window::get_coord(){ // struct XY is declared in Shape_window
  int x, y;
  x = xi.get_int();
  y = yi.get_int();
  
  // when the input is an invalid thing like string, empty, etc, the return
  // value from In_box::get_int() becomes -999999
  if(x==-999999 && y==-999999){ 
    x=0,y=0;
  }
  XY xy{x,y};
  return xy;
}

void Shape_window::add_1Shape(){  
  this->attach(sv[sv.size()-1]);
  //this->attach(*(new Circle{Point{x,y}, 50}));
  // this works fine, but it has a problem that later we cannot delete these
  // dynamically allocated objects, since we cannot later refer to them
  shape_menu.hide();
  shape_menu_bt.show();
}


// Ex 5 of chapter 16
Move_window::Move_window(Point tl, int w, int h, const string& title,
			 const string& file_name)
  : Window{tl, w, h, title},
    quit{Point{x_max()-70, 0}, 70, 20, "Quit",
	 [](Address, Address pw){reference_to<Move_window>(pw).quit_pressed();}},
    next{Point{x_max()-150, 0}, 70, 20, "Next",
	 [](Address, Address pw){reference_to<Move_window>(pw).next_pressed();}},
    ifs{file_name},
    cl{Point{0,0}, 50}
    // since there is not empty constructor for Circle, I need to define it with
    // some parameters. To not show it, I define it first outside of the Window
{
  if(!ifs)
    error("Cannot open file ", file_name);

  attach(quit);
  attach(next);
  attach(cl);
}

// define input operator
istream& operator>>(istream& is, Move_window::XY& xy){
  Move_window::XY buf_xy;
  char ch1, ch2, ch3;
  // assumed format: (x,y)
  if(is >> ch1 >> buf_xy.x >> ch2 >> buf_xy.y >> ch3){
    if(ch1!='(' || ch2!=',' || ch3!=')'){
      is.clear(ios_base::failbit);
      return is;
    }
  }
  else
    return is;			// it's already set to fail() state

  xy = buf_xy;			// xy is a reference
  return is;
}

void Move_window::next_pressed(){
  XY xy;
  // read 1 coordinate pair, and move the Shape
  if(!(ifs >> xy)){
    if(ifs.eof()){
      hide();			// finish the window once reacing end of file
      return;
      // Without return;, the next error() is executed
    }
    error("cannot read this coordinate pair. Format: (x,y)");
  }

  cout << xy.x << " " << xy.y << endl;

  int x,y;
  x = xy.x - cl.center().x;
  y = xy.y - cl.center().y;
  cl.move(x,y);
  redraw();
}


// Ex 6 of chapter 16
Clock_window::Clock_window(Point tl, int w, int h, const string& title)
  : Window{tl, w, h, title},
    quit{Point{x_max()-70, 0}, 70, 20, "Quit",
	 [](Address, Address pw){reference_to<Clock_window>(pw).quit_pressed();}},
    clock{Point{50,50}, "wallclock_without_hands.jpeg"},
    // Since it's not good to be provided with bad images that don't fit the window,
    // I use this pre-determined clock image
    show_clock{Point{x_max()-200, 0}, 100, 20, "Run clock",
	       [](Address, Address pw){reference_to<Clock_window>(pw).show_clock_pressed();}},
    clock_center{Point{50+256,50+253}}, hh_scale{100}, mh_scale{150}, sh_scale{200},
    quit_flag{false}
{
  attach(quit);
  attach(clock);
  attach(show_clock);

  //attach(*(new Arrow{Point{50+256,50+253}, Point{50+256+100,50+253+100}}));

  // I think to show this window, this constructor has to be finished.
  // So I run the clock in a different function, leaving this function (constructor)
  // for now
}

void Clock_window::run_clock(){    
  while(true){			// infinite loop
    one_tick();

    Fl::wait(1);
    // this gives the user more time to push the Quit Button, at the cost of
    // the heads not ticking at each second, but 1 tick in 2~3 seconds
    // Without this, I think the user has to push Quit Button in the exact timing of the
    // program seeing Fl::check(); (== Fl::wait(0);) in one_tick(), which is
    // a little difficult
    
    if(quit_flag)
      break;
    // if I put hide() inside quit_pressed(), on the Quit Button being pressed,
    // hide(); is executed, the Window disappearing, but since the program control
    // is still in this infinite loop, this program keeps running.
    // To avoid that, I introduced this quit_flag, making quit_pressed() just set
    // this flag to true
  }
  hide();
}


void Clock_window::one_tick(){

  auto now = chrono::system_clock::now();
  time_t t = chrono::system_clock::to_time_t(now);
  tm* cal_time = localtime(&t);	// calendar time
  // cal_time->tm_hour gives local hour (in 24 hours), tm_min gives local minutes,
  // and tm_sec gives local seconds
  
  // hour hand
  int ch{cal_time->tm_hour % 12};			// analog clock hour (0~11)
  double hh_rad{(-90+ch*30)*M_PI/180};			// hour hand radian
  // for hour hand, 1 hour = 30 degree
  hh_head.x = clock_center.x + hh_scale* cos(hh_rad);
  hh_head.y = clock_center.y + hh_scale* sin(hh_rad);

  cout << "clock_center= " << clock_center << endl;
  cout << "hh_head= " << hh_head << endl;
    
  // minute hand
  int cm{cal_time->tm_min};			// analog clock min (0~59)
  double mh_rad{(-90+cm*6)*M_PI/180};		// minute hand radian
  // for minute hand, 1 minute = 6 degree
  mh_head.x = clock_center.x + mh_scale* cos(mh_rad);
  mh_head.y = clock_center.y + mh_scale* sin(mh_rad);

  cout << "mh_head= " << mh_head << endl;

  // second hand
  int cs{cal_time->tm_sec};	// analog clock second (0~59)
  double sh_rad{(-90+cs*6)*M_PI/180};
  // for minute hand, 1 second = 6 degree
  sh_head.x = clock_center.x + sh_scale* cos(sh_rad);
  sh_head.y = clock_center.y + sh_scale* sin(sh_rad);

  cout << "sh_head= " << sh_head << endl;
    
  hands.push_back(new Arrow{clock_center, hh_head});		     // hour hand
  hands.push_back(new Arrow{clock_center, mh_head});		// minute hand
  hands.push_back(new Arrow{clock_center, sh_head});		// second hand
  // to detach previously made Arrow objects, I use Vector_ref for Arrows
  // (Without detach, vector<Shape*> shapes in Window class keeps adding garbage
  // addresses/pointers to it)
  // (to do detach(), we need to pass the object, and detach(Shape& s) receives it,
  // and to pass the Arrow objects, we need to store the Arrow objects)

  attach(hands[0]);
  attach(hands[1]);
  attach(hands[2]);

  redraw();
  Fl::check();		// same as Fl::wait(0)
  // without Fl::check(); somehow, the change of the Window state is not reflected
  // on the Window
  sleep(1);			// after 1 sec, draw another positions
    
  detach(hands[0]);
  detach(hands[1]);
  detach(hands[2]);
  hands.clear();
}
  

// Ex 7 of chapter 16
Airplane_window::Airplane_window(Point tl, int w, int h, const string& title,
				 const string& plane_img_file)
  : Window{tl, w, h, title},
    quit{Point{x_max()-70, 0}, 70, 20, "Quit",
	 [](Address, Address pw){reference_to<Airplane_window>(pw).quit_pressed();}},
    start{Point{x_max()-250, 0}, 70, 20, "Start",
	 [](Address, Address pw){reference_to<Airplane_window>(pw).start_pressed();}},
    stop{Point{x_max()-170, 0}, 70, 20, "Stop",
	 [](Address, Address pw){reference_to<Airplane_window>(pw).stop_pressed();}},
    apl_img{Point{0,0}, plane_img_file},
    quit_flag{false},
    theta1{90}, theta2{270}, c1_flag{false}, img_h{65},
    c1{Point{x_max()/2-50, y_max()/4}}, // -50 to adjust the position for the image width
    c2{Point{x_max()/2-50, 3*y_max()/4-img_h/2}}, radius{y_max()/4}
{
  attach(quit);
  attach(start);
  attach(stop);

  // Image apl_img{Point{0,0}, plane_img_file};
  // This Point can be anything, because I don't use the info to draw the image.
  // This Image class is to read the image in appropriate mode (either JPEG or GIF),.
  // e.g. p = new Fl_JPEG_Image(s.c_str());
  // As shown in the above line, to read an image with FLTK, we don't provide
  // location information. The location where the image is drawn is decided in
  // Imgage::draw_lines(), where  p->draw(point(0).x,point(0).y,w,h,cx,cy); happens.
  // <- if I define apl_img in this constructor, when the program gets out of this
  //    function, apl_img is deleted, so Window cannot show the image, and
  //    seg fault happens.

  attach(apl_img);
  apl_img.move(x_max()/2-50, y_max()/2);
  apl_img.resize(105,65);
  //p = new Fl_PNG_Image(plane_img_file.c_str());
   
  //p = apl_img.get_Fl_image_pt()->copy(100,100);
  //p = apl_img.get_Fl_image_pt();
  // p->draw(100,100);
  // I don't know why, but this approach didn't work, maybe due to the timing of calling
  // p->draw();. If Image is attached to a Window, p->draw() is called when
  // Window::draw() is called, but I don't know when this is called.
}

void Airplane_window::start_pressed(){
  stop_flag = false;		// initialize it
  // But quit_flag shouldn't be initialized, because once it is pressed, it should
  // have a permanent power to break infinite loops in start_pressed() and suspend()
  
  // the same structure as Clock_window::run_clock()
  while(true){
    move_airplane();
    Fl::check();		// same as Fl::wait(0);, it checks if any Button is pressed
    if(quit_flag || stop_flag)
      break;
  }

  if(quit_flag)
    hide();			// reached only when quit_flag is true
  else			    
    suspend();// reached only when stop_flag is true, but quit_flag is false
}

// move the airplane picture a bit
void Airplane_window::move_airplane(){
  if(c1_flag){
    move_on_c1();
  }
  else
    move_on_c2();
  
  sleep(1);
  redraw();
}

// move the airplane image a bit on Circle 1 (the upper Circle)
void Airplane_window::move_on_c1(){
  theta1 += 10;			// move in unit of 10 degrees
  Point new_pos;
  new_pos.x = c1.x + radius*cos(theta1*M_PI/180);
  new_pos.y = c1.y + radius*sin(theta1*M_PI/180);
  
  apl_img.position(new_pos);

  if(theta1 == 450){		// when the airplane moves 1 round on Circle 1
    theta1 = 90;		// initialize to its initial angle
    c1_flag = false;
  }
}

// move the airplane image a bit on Circle 2 (the lower Circle)
void Airplane_window::move_on_c2(){
  theta2 -= 10;	   // move in unit of 10 degrees, in the other direction as on Circle 1
  Point new_pos;
  new_pos.x = c2.x + (radius-img_h/2)*cos(theta2*M_PI/180);
  new_pos.y = c2.y + (radius-img_h/2)*sin(theta2*M_PI/180);
  // adjust the radius of Circle 2, so that the image is not out of the Window
  
  apl_img.position(new_pos);

  if(theta2 == -90){		// when the airplane moves 1 round on Circle 2
    theta2 = 270;		// initialize to its initial angle
    c1_flag = true;
  }
}


// wait until any Button is pushed
void Airplane_window::suspend(){
  // to go back to an infinite loop after the user presses "Stop" while in
  // suspend(), I use 2 while loops
  //cout << "### 1\n";
  while(true){
    //cout << "### 2\n";
    Fl::check();
    // keep iterating infinitely until some Button is pressed
    // If Quit is pressed, quit_flag becomes true, and program breaks this infinite loop
    // If Start is pressed, start_pressed() is called, and another infinite loop
    //   in start_pressed() starts inside this infinite loop.
    // If Stop is pressed, stop_flag becomes true, (which is already true), but nothing
    //   other than that happens, and stay inside this infinite loop.
    
    if(quit_flag)
      break;
  }
  hide();			// reached only when quit_flag is true
}


//------------------------------------------------------------------------------

//------------------------------------------------------------------------------

Function_window::Function_window(Point tl, int w, int h, const string& title)
  : Window{tl, w, h, title},
    quit{Point{x_max()-70,0}, 70, 20, "Quit",
	 [](Address, Address wp){reference_to<Function_window>(wp).quit_pressed();}},
    a_in{Point{x_max()-400,0}, 70, 20, "a:"}, b_in{Point{x_max()-300,0}, 70, 20, "b:"},
    c_in{Point{x_max()-200,0}, 70, 20, "c:"},
    func_menu{Point{x_max()-100,60}, 100, 20, Menu::vertical, "functions:"},
    xscale{10}, yscale{10},	// 1 notch = 10 pixels, so the scale is 10
    r_min{-25}, r_max{25},
    // r_min/max is the number of notches on the left of y Axis and on the right of it,
    // respectively. Not the number of pixels.
    orig{Point{x_max()/2, y_max()/2}},
    x{Axis::x, Point{50, y_max()/2}, 500, 50, "x (1 unit=10 pixels)"},
    y{Axis::y, Point{x_max()/2, y_max()-50}, 500, 50, "y (1 unit=10 pixels)"},
    clear{Point{x_max()-70,y_max()-30}, 70, 20, "Clear",
	 [](Address, Address wp){reference_to<Function_window>(wp).clear_pressed();}}
{
  attach(quit);
  attach(a_in);   attach(b_in);  attach(c_in);

  func_menu.attach(new Button{Point{0,0}, 0, 0, "a*sin(b*x+c)", cb_sin});
  // since Buttons attached to a Menu are adjusted Menu's position, width, and height,
  // these parameters for attached Buttons can be anything
  func_menu.attach(new Button{Point{0,0}, 0, 0, "a*log(b*x+c)", cb_log});
  func_menu.attach(new Button{Point{0,0}, 0, 0, "a*x+b", cb_linear});
  func_menu.attach(new Button{Point{0,0}, 0, 0, "a*x^2+b*x+c", cb_parabola});
  attach(func_menu);
  
  attach(x); attach(y);

  attach(clear);
}

void Function_window::get_param(){
  a = a_in.get_int(); b = b_in.get_int(); c = c_in.get_int(); // accepts only int
}

void Function_window::sin_pressed(){
  get_param();			// get a, b, c
  fv.push_back(new Function{[this](double x){return a*sin(b*x+c);},
			      static_cast<double>(r_min), static_cast<double>(r_max), orig,
			      100,
			      static_cast<double>(xscale), static_cast<double>(yscale)});
  // sin() is from cmath library, and cmath is included in Graph.h, which is included
  // in GUI.h, so it is usable
  // If I want to use member variables inside lambda functions, capture "this" in [],
  // not the member variables themselves. If I do the latter, error
  // "error: 'a' in capture list does not name a variable" happens
  attach(fv[fv.size()-1]);
  redraw();
}

void Function_window::log_pressed(){
  get_param();			// get a, b, c
  fv.push_back(new Function{[this](double x){
			      if(b*x+c>0) return a*log(b*x+c);
			      else return static_cast<double>(0);},
			      static_cast<double>(r_min), static_cast<double>(r_max), orig,
			      100,
			      static_cast<double>(xscale), static_cast<double>(yscale)});
  attach(fv[fv.size()-1]);
  redraw();
}

void Function_window::linear_pressed(){
  get_param();			// get a, b, c
  fv.push_back(new Function{[this](double x){return a*x+b;},
			      static_cast<double>(r_min), static_cast<double>(r_max), orig,
			      100,
			      static_cast<double>(xscale), static_cast<double>(yscale)});
  attach(fv[fv.size()-1]);
  redraw();
}

void Function_window::parabola_pressed(){
  get_param();			// get a, b, c
  fv.push_back(new Function{[this](double x){return a*x*x + b*x + c;},
			      static_cast<double>(r_min), static_cast<double>(r_max), orig,
			      100,
			      static_cast<double>(xscale), static_cast<double>(yscale)});
  attach(fv[fv.size()-1]);
  redraw();
}

void Function_window::clear_pressed(){
  // detach all the Functions
  for(int i=0; i<fv.size(); ++i)
    detach(fv[i]);
  fv.clear();
  redraw();
}

}; // of namespace Graph_lib
