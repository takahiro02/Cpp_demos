
//
// This is a GUI support code to the chapters 12-16 of the book
// "Programming -- Principles and Practice Using C++" by Bjarne Stroustrup
//

#ifndef GUI_GUARD
#define GUI_GUARD

#include "Window.h"
#include "Graph.h" // needed for Vector_ref
#include <FL/Fl_Button.H>
// for using Fl_Button type in Button::set() and Button::clear()
#include <FL/Fl_Tiled_Image.H>

// for sleep() function (https://www.softwaretestinghelp.com/cpp-sleep/) for Ex 6
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

#include <FL/Fl_PNG_Image.H>	// Ex 7

#include<chrono>

namespace Graph_lib {

//------------------------------------------------------------------------------

    typedef void* Address;    // Address is a synonym for void*
    typedef void(*Callback)(Address, Address);    // FLTK's required function type for all callbacks
  // this looks like we define a function pointer whose return type is void and
  // who takes 2 arguments of type Address as Callback
  // <- this view is correct. https://stackoverflow.com/questions/4295432/
  // So Callback do_it; means void(*do_it)(Address, Address); (a function pointer named do_it).
  // The concept "function pointer" is not explained in the book. See the site
  // https://www.cprogramming.com/tutorial/function-pointers.html
  // A function pointer is, as the name suggests, a pointer to a function. It stores the address
  // of the function definition (function definitions are stored somewhere in memory (I don't
  // know whether the location is stack, or static)), and
  // we can call the pointed function using the function pointer, without dereferencing
  // (*func_ptr() <- * is not needed). See the easy examples on the website in
  // "Initializing Function Pointers" and "Using a Function Pointer" sections.
  // In this file, the function pointer Callback is used by explicitly naming a callback
  // function, or directly giving it an anonimous function, such as
  // [](Address, Address pw){reference_to<Airplane_window>(pw).quit_pressed();} (From GUI.cpp,
  // class Airplane_window's constructor)
  
//------------------------------------------------------------------------------

    template<class W> W& reference_to(Address pw)
    // treat an address as a reference to a W
    {
        return *static_cast<W*>(pw);
    }

//------------------------------------------------------------------------------


    class Widget {
    // Widget is a handle to an Fl_widget - it is *not* an Fl_widget
    // We try to keep our interface classes at arm's length from FLTK
    public:
        Widget(Point xy, int w, int h, const string& s, Callback cb)
            : loc(xy), width(w), height(h), label(s), do_it(cb)
        {}

        virtual ~Widget() { }
        
        virtual void move(int dx,int dy) { hide(); pw->position(loc.x+=dx, loc.y+=dy); show(); }
        virtual void hide() { pw->hide(); }
        virtual void show() { pw->show(); }
        virtual void attach(Window&) = 0;

        Point loc;
        int width;
        int height;
        string label;
        Callback do_it;
      // the same as void(*do_it)(Address, Address); (function pointer named do_it)

      // Ex 3 of Chapter 16
      void position(int x, int y){
	loc.x = x, loc.y = y;	// 1st, update the current location
	pw->hide(); pw->position(x,y); pw->show();}
      // Using Fl_Widget::position(), reposition the widget.
      // Unlike Widget::move() defined above, position() moves Widget based on the
      // relative position to the Window, not relative to its previous position.
      // Without pw->hide(), previous Widget won't disappear.
      void image(Image& img){
	pw->image(img.get_Fl_image_pt()->copy(width, height));
      }
      // Using Fl_Widget::image(Fl_image*), set image as part of the widget label
      
    protected:
        Window* own;    // every Widget belongs to a Window
        Fl_Widget* pw;  // connection to the FLTK Widget

    };


//------------------------------------------------------------------------------

class Button : public Widget {
public:
	Button(Point xy, int w, int h, const string& label, Callback cb)
            : Widget(xy,w,h,label,cb)
        {}

        void attach(Window& win);

  // Ex 2 of chapter 16
  void set_color(Fl_Color c){pw->color(c);}
  // since pw is Fl_Widget* type, we cannot use member functions unique to Fl_Button type.
  // We can only use member functions defined for Fl_Widget type
  // For what functions can be used for Fl_Widget, see
  // https://www.fltk.org/doc-1.3/classFl__Widget.html
  
  void set(){reference_to<Fl_Button>(pw).set();}	// use Fl_Button::set()
  void clear(){reference_to<Fl_Button>(pw).clear();}	// use Fl_Button::clear() 
};

//------------------------------------------------------------------------------

    struct In_box : Widget {
        In_box(Point xy, int w, int h, const string& s)
            :Widget(xy,w,h,s,0) { }
        int get_int();
        string get_string();

        void attach(Window& win);
    };

//------------------------------------------------------------------------------

    struct Out_box : Widget {
        Out_box(Point xy, int w, int h, const string& s)
            :Widget(xy,w,h,s,0) { }
        void put(int);
        void put(const string&);

        void attach(Window& win);
    };
//------------------------------------------------------------------------------
    
struct Menu : Widget {
        enum Kind { horizontal, vertical };
        Menu(Point xy, int w, int h, Kind kk, const string& label);
  // Menu's width and height are the width and height of Buttons to be added in the
  // future
  
        Vector_ref<Button> selection;
        Kind k;
        int offset;
        int attach(Button& b);      // Menu does not delete &b
        int attach(Button* p);      // Menu deletes p

        void show()                 // show all buttons
        {
            for (unsigned int i = 0; i<selection.size(); ++i)
                selection[i].show();
        }
        void hide();                 // hide all buttons
       
        void move(int dx, int dy);   // move all buttons

        void attach(Window& win);    // attach all buttons

};

//------------------------------------------------------------------------------

  /* // for Drill 2
  struct Lines_window : Window {
    Lines_window(Point xy, int w, int h, const string& title);
    Open_polyline lines;
  private:
    // widgets
    Button next_button;
    Button quit_button;
    In_box next_x;
    In_box next_y;
    Out_box xy_out;

    void next();
    void quit(){hide();}
  };
  */
  
  // for Drill 3, 4
  // from p573-575
  // Note: without wait_for_button() like Simple_window class, how the program waits
  // for the next user reponse? -> it waits in gui_main() defined in Window.cpp
  struct Lines_window : Window {
    Lines_window(Point xy, int w, int h, const string& title);
  private:
    Open_polyline lines;

    // widgets
    Button next_button;
    Button quit_button;
    In_box next_x;
    In_box next_y;
    Out_box xy_out;
    Menu color_menu;
    Button menu_button;

    Menu style_menu;
    Button style_menu_button;
    
    void change(Color c){lines.set_color(c);}
    void change_style(Line_style ls){lines.set_style(ls);}

    void hide_menu(){color_menu.hide(); menu_button.show();}
    void hide_style_menu(){style_menu.hide(); style_menu_button.show();}

    // actions invoked by callbacks
    void red_pressed(){change(Color::red); hide_menu();}
    void blue_pressed(){change(Color::blue); hide_menu();}
    void black_pressed(){change(Color::black); hide_menu();}
    void menu_pressed(){menu_button.hide(); color_menu.show();}
    void next();
    void quit(){hide();}

    void dash_pressed(){change_style(Line_style::dash); hide_style_menu();}
    void solid_pressed(){change_style(Line_style::solid); hide_style_menu();}
    void dot_pressed(){change_style(Line_style::dot); hide_style_menu();}
    void style_menu_pressed(){style_menu_button.hide(); style_menu.show();}
    
    // callback functions (for each Button class, a callback function is prepared):
    static void cb_red(Address, Address pw)
    {reference_to<Lines_window>(pw).red_pressed();}
    static void cb_blue(Address, Address pw)
    {reference_to<Lines_window>(pw).blue_pressed();}
    static void cb_black(Address, Address pw)
    {reference_to<Lines_window>(pw).black_pressed();}
    static void cb_menu(Address, Address pw){reference_to<Lines_window>(pw).menu_pressed();}
    static void cb_next(Address, Address pw){reference_to<Lines_window>(pw).next();}
    static void cb_quit(Address, Address pw){reference_to<Lines_window>(pw).quit();}

    static void cb_dash(Address, Address pw)
    {reference_to<Lines_window>(pw).dash_pressed();}
    static void cb_solid(Address, Address pw)
    {reference_to<Lines_window>(pw).solid_pressed();}
    static void cb_dot(Address, Address pw)
    {reference_to<Lines_window>(pw).dot_pressed();}
    static void cb_style_menu(Address, Address pw)
    {reference_to<Lines_window>(pw).style_menu_pressed();}
  };

  
  // Ex 1 and 2 of chapter 16
  // difference of My_window from Simple_window is just that My_window has quit button
  struct My_window : Window {
    My_window(Point tl, int w, int h, const string& title);
    // tl: top left corner of the window

  private:
    // Widgets
    Button next;
    Button quit;

    Menu checkboard1;
    Menu checkboard2;
    Menu checkboard3;
    Menu checkboard4;
    // since a Menu can grow in either horizontal or vertical direction, we need
    // 4 Menus to make a 4-by-4 checkboard

    // since there is gui_main() in main.cpp, I don't need a wait function (in gui_main(),
    // it calls Fl::run(), and Fl::run() calls Fl::wait(), which is called in
    // Simple_window::wait_for_button() too)
    void next_pressed(){clear_all_cbbuttons();}	// from FLTK library
    void quit_pressed(){hide();}

    // different actions for each Button on the checkboard
    void checkboardm1b1_pressed(){ // 1st Button in checkboard1
      checkboard_push_only_1(1,1);
    }
    void checkboardm1b2_pressed(){checkboard_push_only_1(1,2);}
    void checkboardm1b3_pressed(){checkboard_push_only_1(1,3);}
    void checkboardm1b4_pressed(){checkboard_push_only_1(1,4);}
    void checkboardm2b1_pressed(){checkboard_push_only_1(2,1);}
    void checkboardm2b2_pressed(){checkboard_push_only_1(2,2);}
    void checkboardm2b3_pressed(){checkboard_push_only_1(2,3);}
    void checkboardm2b4_pressed(){checkboard_push_only_1(2,4);}
    void checkboardm3b1_pressed(){checkboard_push_only_1(3,1);}
    void checkboardm3b2_pressed(){checkboard_push_only_1(3,2);}
    void checkboardm3b3_pressed(){checkboard_push_only_1(3,3);}
    void checkboardm3b4_pressed(){checkboard_push_only_1(3,4);}
    void checkboardm4b1_pressed(){checkboard_push_only_1(4,1);}
    void checkboardm4b2_pressed(){checkboard_push_only_1(4,2);}
    void checkboardm4b3_pressed(){checkboard_push_only_1(4,3);}
    void checkboardm4b4_pressed(){checkboard_push_only_1(4,4);}

    void checkboard_push_only_1(int menu_i, int button_i);
    // function used in all the checkboardmibj_pressed() above
    void clear_all_cbbuttons();
    // clear all the checkboard Buttons

    //static void cb_checkboard(Address bp, Address pw);
    // bp points to the Fl_Button object created in Button::attach()
    // For some mysterious reason, without static, compile error happens
    // And since lambda functions are also accepted, it seems they are also treated as
    // static?
    // <- But according to
    //    https://medium.com/@winwardo/c-lambdas-arent-magic-part-2-ce0b48934809
    //    lambda functions are allocated to stack memory, which is different from
    //    static memory (for the difference of static memory, heap memory, and
    //    stack memory, see https://stackoverflow.com/questions/408670/).
    //    So I don't know why Fl_Widget::callback() accepts lambda functions as well.
    // I decided to pass different callback functions unique to each Button to
    // the Button objects of the checkboard, since each Button needs to behave differently
    // (I can still do this with passing the same callback function to all the Buttons
    // of the checkboard, but to do so, I need to pass the Button object address to the
    // 2nd argument of Fl_Button::callback() in Button::attach(), and which makes it
    // necessary to define all possible Button functionalities inside Button class, for
    // the reasons mentioned in the comments in Button::attach(), and
    // My_window::cb_checkboard().)

    // callback functions for each of the checkboard Button objects
    static void cb_m1b1(Address, Address wp)
    {reference_to<My_window>(wp).checkboardm1b1_pressed();}
    static void cb_m1b2(Address, Address wp)
    {reference_to<My_window>(wp).checkboardm1b2_pressed();}
    static void cb_m1b3(Address, Address wp)
    {reference_to<My_window>(wp).checkboardm1b3_pressed();}
    static void cb_m1b4(Address, Address wp)
    {reference_to<My_window>(wp).checkboardm1b4_pressed();}
    static void cb_m2b1(Address, Address wp)
    {reference_to<My_window>(wp).checkboardm2b1_pressed();}
    static void cb_m2b2(Address, Address wp)
    {reference_to<My_window>(wp).checkboardm2b2_pressed();}
    static void cb_m2b3(Address, Address wp)
    {reference_to<My_window>(wp).checkboardm2b3_pressed();}
    static void cb_m2b4(Address, Address wp)
    {reference_to<My_window>(wp).checkboardm2b4_pressed();}
    static void cb_m3b1(Address, Address wp)
    {reference_to<My_window>(wp).checkboardm3b1_pressed();}
    static void cb_m3b2(Address, Address wp)
    {reference_to<My_window>(wp).checkboardm3b2_pressed();}
    static void cb_m3b3(Address, Address wp)
    {reference_to<My_window>(wp).checkboardm3b3_pressed();}
    static void cb_m3b4(Address, Address wp)
    {reference_to<My_window>(wp).checkboardm3b4_pressed();}
    static void cb_m4b1(Address, Address wp)
    {reference_to<My_window>(wp).checkboardm4b1_pressed();}
    static void cb_m4b2(Address, Address wp)
    {reference_to<My_window>(wp).checkboardm4b2_pressed();}
    static void cb_m4b3(Address, Address wp)
    {reference_to<My_window>(wp).checkboardm4b3_pressed();}
    static void cb_m4b4(Address, Address wp)
    {reference_to<My_window>(wp).checkboardm4b4_pressed();}
    
  };

  // Ex 3 of chapter 16
  struct Image_window : Window {
    // version 1
    //Image_window(Point tl, int w, int h, const string& title);
    // tl: top left corner of the window

    // version 2
    Image_window(Point tl, int w, int h, const string& title, const string& img_name,
		 Point xy=Point{0,0}, int iw=100, int ih=100);
    // xy: top left corner of the mask of the image, iw: masked image's width,
    // ih: masked image's height

    void set_image(Image& img){
      image_b.image(img);
    }
  private:
    // Widgets
    Button quit;

    Button image_b;
    Image image;
    
    // since there is gui_main() in main.cpp, I don't need a wait function (in gui_main(),
    // it calls Fl::run(), and Fl::run() calls Fl::wait(), which is called in
    // Simple_window::wait_for_button() too)
    void quit_pressed(){hide();}

    // image_pressed() for version 1
    /*
    void image_pressed(){
      image_b.position(rand_int(0, x_max()-image_b.width),
		       rand_int(0, y_max()-image_b.height));
    }
    */

    // image_pressed() for version 2
    void image_pressed();
    
  };


  // Ex 4
  struct Shape_window : Window {    
    Shape_window(Point tl, int w, int h, const string& title);
    // tl: top left corner of the window
  private:
    // Widgets
    Button quit;
    Menu shape_menu;
    Button shape_menu_bt;
    In_box xi;
    In_box yi;

    Vector_ref<Shape> sv;
    // we can attach a bare dynamically allocated object to a Window, by, e.g.
    // win.attach(*(new Circle{Point{x,y},50}));
    // But this way has the problem that later we cannot refer back to these
    // dynamically allocated objects, which means later in destructor for example,
    // we cannot destroy these dynamically allocated objects. So I will store
    // the pointers of the dynamically allocated objects with this Vector_ref

    // Since Vector_ref has its own destructor that deletes all the dynamically
    // allocated elements, I don't have to do that in this class, for sv,
    // and for any Menu class
    
    // since there is gui_main() in main.cpp, I don't need a wait function (in gui_main(),
    // it calls Fl::run(), and Fl::run() calls Fl::wait(), which is called in
    // Simple_window::wait_for_button() too)
    void quit_pressed(){hide();}

    static void cb_shapemenu(Address, Address wp){
      reference_to<Shape_window>(wp).shapemenu_pressed();
    }
    void shapemenu_pressed(){shape_menu_bt.hide(); shape_menu.show();}

    
    static void cb_circle(Address, Address wp){
      // wp: window pointer (points to this Shape_window object itself)
      reference_to<Shape_window>(wp).circle_pressed();
      // I need to cast the wp pointer from void* to Shape_window* to call
      // a member function inside this class
    }
    static void cb_square(Address, Address wp){
      reference_to<Shape_window>(wp).square_pressed();
    }
    static void cb_equi_tri(Address, Address wp){
      reference_to<Shape_window>(wp).equi_tri_pressed();
    }
    static void cb_hexagon(Address, Address wp){
      reference_to<Shape_window>(wp).hexagon_pressed();
    }

    struct XY{int x, y;};	// for receiving the inputs
    XY get_coord();
    
    void circle_pressed(){
      XY xy{get_coord()};
      sv.push_back(new Circle{Point{xy.x,xy.y}, 50});
      // thanks to #include"Graph.h" in this file, we can use those Shape classes
      add_1Shape();
    }
    void square_pressed(){
      XY xy{get_coord()};
      sv.push_back(new Rectangle{Point{xy.x-25,xy.y-25}, 50, 50});
      add_1Shape();
    }
    void equi_tri_pressed(){
      XY xy{get_coord()};
      // add an equilateral triangle whose center is xy
      sv.push_back(new Closed_polyline{{Point{xy.x-25,xy.y+static_cast<int>(25/sqrt(3))},
	      Point{xy.x,xy.y-static_cast<int>(50/sqrt(3))},
		Point{xy.x+25,xy.y+static_cast<int>(25/sqrt(3))}}});
      add_1Shape();
    }
    void hexagon_pressed(){
      XY xy{get_coord()};
      sv.push_back(new Regular_polygon{Point{xy.x,xy.y}, 6, 25});
      add_1Shape();
    }

    void add_1Shape();
    // extract the common procedures to add a Shape object to the Menu
  };


  // Ex 5 of chapter 16
  struct Move_window : Window {    
    Move_window(Point tl, int w, int h, const string& title, const string& file_name);
    // tl: top left corner of the window
    // The Shape moves according to the coordinates written in the file name
    struct XY{int x, y;};	// for receiving the inputs
    // for using this XY class outside of this class (in GUI.cpp,
    // istream& operator>>(istream& is, Move_window::XY& xy)), I declare this class
    // in public area (If it is in private, even if I call this with Move_window::XY,
    // an error happens)
  private:
    // Widgets
    Button quit;
    Button next;

    ifstream ifs;

    Circle cl;
    
    // since there is gui_main() in main.cpp, I don't need a wait function (in gui_main(),
    // it calls Fl::run(), and Fl::run() calls Fl::wait(), which is called in
    // Simple_window::wait_for_button() too)
    void quit_pressed(){hide();}
    void next_pressed();
  };


  // Ex 6 of chapter 16
  struct Clock_window : Window {    
    Clock_window(Point tl, int w, int h, const string& title);
    // tl: top left corner of the window
    
  private:
    // Widgets
    Button quit;
    Button show_clock;

    Image clock;
    // the size of the clock image is 512 * 506 (obtained from seeing the image info)
    const Point clock_center;
    const int hh_scale;		// hour hand scale
    const int mh_scale;		// minute hand scale
    const int sh_scale;		// second hand scale

    Point hh_head;		// hour hand head
    Point mh_head;		// minute hand head
    Point sh_head;		// second hand head

    Vector_ref<Arrow> hands;

    bool quit_flag;

    // since there is gui_main() in main.cpp, I don't need a wait function (in gui_main(),
    // it calls Fl::run(), and Fl::run() calls Fl::wait(), which is called in
    // Simple_window::wait_for_button() too)
    void quit_pressed(){quit_flag=true;}
    void show_clock_pressed(){run_clock();}

    void run_clock();
    void one_tick();		// curve 1 tick in run_clock()
  };


  // Ex 7 of chapter 16
  struct Airplane_window : Window {
    Airplane_window(Point tl, int w, int h, const string& title, const string& plane_img_file);
    // tl: top left corner of the window
    
  private:
    // Widgets
    Button quit;
    Button start, stop;
    // start/stop the move of the airplane image

    //Button plane_button;
    // since in FLTK, there is no function to resize an image, I use the feature
    // I found in Exercise 3 of chapter 16 that when an image is used as an image label
    // to a Fl_Widget, its size is resized as done 
    Image apl_img;
    const int img_h;		// image height, used for adjusting radius of Circle 2
    
    bool quit_flag, stop_flag;

    int theta1, theta2;		// degree (not radian) of the position of the 
    // I decided to make the airplane circle on 2 Circles, and that's controlled by
    // these 2 radians
    Point c1, c2;	// centers of the 2 Circles
    int radius;		// radiuses of these 2 Circles. I set them to be the same
    bool c1_flag;	// true if the upper Circle is moving
    void move_on_c1();	// move the airplane image a bit on Circle 1 (the upper Circle)
    void move_on_c2();	// move the airplane image a bit on Circle 2 (the lower Circle)
      
    //Fl_Image* p;
    // I first tried to draw from Fl_Image* in the same procedures as those in
    // Image::draw_lines(), but that didn't succeed, maybe due to the timing of
    // calling p->draw(...) (I don't know when the FLTK program, while(Fl::wait());,
    // calls Window::draw(), which in turn calls Image::draw(), which calls p->draw(...)).
    // So I added Image::resize() function to Image class, which worked as I expected

    // since there is gui_main() in main.cpp, I don't need a wait function (in gui_main(),
    // it calls Fl::run(), and Fl::run() calls Fl::wait(), which is called in
    // Simple_window::wait_for_button() too)
    void quit_pressed(){quit_flag=true;}

    void start_pressed();
    void stop_pressed(){stop_flag=true;}

    void suspend();
    void move_airplane();
  };

  // Exercise 10 of chapter 16
  struct Function_window : Window {
    Function_window(Point tl, int w, int h, const string& title);
    // tl: top left corner of the window
    
  private:
    // Widgets
    Button quit;
    In_box a_in, b_in, c_in;		// parameters of Functions
    Menu func_menu;		// stores Buttons for options of functions
    Button clear;		// clear all attached Functions

    void get_param();		// get a, b, c from the In_boxes
    int a, b, c;
    
    Axis x, y;
    // the variables below are for defining Axis and Functions
    int xscale, yscale;
    int r_min, r_max;
    Point orig;

    Vector_ref<Function> fv;	// store functions to be displayed

    // since there is gui_main() in main.cpp, I don't need a wait function (in gui_main(),
    // it calls Fl::run(), and Fl::run() calls Fl::wait(), which is called in
    // Simple_window::wait_for_button() too)
    void quit_pressed(){hide();}

    void clear_pressed();
    
    void sin_pressed();
    void log_pressed();
    void linear_pressed();
    void parabola_pressed();

    static void cb_sin(Address, Address wp)
    {reference_to<Function_window>(wp).sin_pressed();};
    static void cb_log(Address, Address wp)
    {reference_to<Function_window>(wp).log_pressed();};
    static void cb_linear(Address, Address wp)
    {reference_to<Function_window>(wp).linear_pressed();};
    static void cb_parabola(Address, Address wp)
    {reference_to<Function_window>(wp).parabola_pressed();};
  };
  

} // of namespace Graph_lib

#endif // GUI_GUARD
