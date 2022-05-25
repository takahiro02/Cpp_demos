//
// This is a GUI support code to the chapters 12-16 of the book
// "Programming -- Principles and Practice Using C++" by Bjarne Stroustrup
//

#ifndef GRAPH_GUARD
#define GRAPH_GUARD 1

#include <FL/fl_draw.H>
#include <FL/Fl_Image.H>
#include "Point.h"
#include "std_lib_facilities.h"
#include <functional>
#include<cmath>			// for sqrt(), pow() in Arrow::Arrow()

#include<algorithm>

namespace Graph_lib {

// defense against ill-behaved Linux macros:
#undef major
#undef minor

  //------------------------------------------------------------------------------
  // helper function, which doesn't belong to any class
  inline double deg2rad(double deg){
    return M_PI * (deg/180.0);
    // M_PI is defined in cmath.h
  }
  // It seems inline function should be defined in header files. It is defined in multiple
  // translation units.
  // https://stackoverflow.com/questions/1759300/
  
//------------------------------------------------------------------------------

// Color is the type we use to represent color. We can use Color like this:
//    grid.set_color(Color::red);
struct Color {
    enum Color_type {
        red=FL_RED,
        blue=FL_BLUE,
        green=FL_GREEN,
        yellow=FL_YELLOW,
        white=FL_WHITE,
        black=FL_BLACK,
        magenta=FL_MAGENTA,
        cyan=FL_CYAN,
        dark_red=FL_DARK_RED,
        dark_green=FL_DARK_GREEN,
        dark_yellow=FL_DARK_YELLOW,
        dark_blue=FL_DARK_BLUE,
        dark_magenta=FL_DARK_MAGENTA,
        dark_cyan=FL_DARK_CYAN
    };

    enum Transparency { invisible = 0, visible=255 };

    Color(Color_type cc) :c(Fl_Color(cc)), v(visible) { }
    Color(Color_type cc, Transparency vv) :c(Fl_Color(cc)), v(vv) { }
    Color(int cc) :c(Fl_Color(cc)), v(visible) { }
  Color(int cc, Transparency vv) : c(Fl_Color(cc)), v(vv) {} // Ex 16 of Ch14
    Color(Transparency vv) :c(Fl_Color{}), v(vv) { }    // default color
  Color(int, int, int);					// exercise 7
  
    int as_int() const { return c; }

    char visibility() const { return v; } 
    void set_visibility(Transparency vv) { v=vv; }
private:
    char v;    // invisible and visible for now
    Fl_Color c;
};

//------------------------------------------------------------------------------

struct Line_style {
    enum Line_style_type {
        solid=FL_SOLID,            // -------
        dash=FL_DASH,              // - - - -
        dot=FL_DOT,                // ....... 
        dashdot=FL_DASHDOT,        // - . - . 
        dashdotdot=FL_DASHDOTDOT,  // -..-..
    };

    Line_style(Line_style_type ss) :s(ss), w(0) { }
    Line_style(Line_style_type lst, int ww) :s(lst), w(ww) { }
    Line_style(int ss) :s(ss), w(0) { }

    int width() const { return w; }
    int style() const { return s; }
private:
    int s;
    int w;
};

//------------------------------------------------------------------------------

class Font {
public:
    enum Font_type {
        helvetica=FL_HELVETICA,
        helvetica_bold=FL_HELVETICA_BOLD,
        helvetica_italic=FL_HELVETICA_ITALIC,
        helvetica_bold_italic=FL_HELVETICA_BOLD_ITALIC,
        courier=FL_COURIER,
        courier_bold=FL_COURIER_BOLD,
        courier_italic=FL_COURIER_ITALIC,
        courier_bold_italic=FL_COURIER_BOLD_ITALIC,
        times=FL_TIMES,
        times_bold=FL_TIMES_BOLD,
        times_italic=FL_TIMES_ITALIC,
        times_bold_italic=FL_TIMES_BOLD_ITALIC,
        symbol=FL_SYMBOL,
        screen=FL_SCREEN,
        screen_bold=FL_SCREEN_BOLD,
        zapf_dingbats=FL_ZAPF_DINGBATS
    };

    Font(Font_type ff) :f{ff} { }
    Font(int ff) :f{ff} { }

    int as_int() const { return f; }
private:
    int f;
};

//------------------------------------------------------------------------------

template<class T> class Vector_ref {
    vector<T*> v;		// vector of pointers of type T (any type)
    vector<T*> owned;		// vector used for unnamed objects (e.g. new Rectangle{...})
  // vector of like T* t = new Rectangle{...}
  // This naming "owned" seems to reflect the fact that unnamed objects are owned by this
  // vector, whereas named objects are not owned in this class, and can be out of control
  // of this class
  // Why did the authour of Vector_ref class separate v and owned?
  // -> I think that's because separating them makes it easier to delete unnamed objects,
  //    as in the destructor and clear() in this class.
  // (when we try to delete a named object, it generates an error as follows:
  // "malloc: *** error for object 0x7ff7b4fd2348: pointer being freed was not allocated")
  // Since v stores both named and unnamed objects, whereas owned stores only unnamed
  // ones, the numbers of elements of them can be different once named objects are
  // pointed to.
  
  //vector<const T*> test;
public:
    Vector_ref() {}

  ~Vector_ref() { for (int i=0; i<owned.size(); ++i){ delete owned[i]; }}
  // since dynamically allocated memory that is stored in this Vector_ref is
  // only in "owned", free them ("v" is storing addresses of named objects, as
  // well as unnamed ones. we cannot free memories of named objects, since their
  // scope might be different from that of this Vector_ref object)

    void push_back(T& s) { v.push_back(&s); } // for named object
  // e.g. Circle c{...};
  // this stores the pointer to the named object, &s (address of s)
    void push_back(T* p) { v.push_back(p); owned.push_back(p); }
  // for unnamed object, e.g. new Circle{...};
  //void push_back(const T& s){test.push_back(&s);}

    T& operator[](int i) { return *v[i]; }
  // return the stored content, like T* t = new ...; *t;
    const T& operator[](int i) const { return *v[i]; }

    int size() const { return v.size(); }

  // delete dynamically allocated memories (stored in "owned"), and
  // make the vectors of pointers empty
  void clear(){
    for(int i=0; i<owned.size(); ++i){
      delete owned[i];
    }
    // still owned and v are storing pointers (addresses) to the now empty
    // memories, so remove them as well
    v.clear(), owned.clear();
  }

  // I tried to implement erase() member function, but I quit, since the number of
  // elements of "v" and "owned" can be different (since v stores both named and unnamed
  // objects, whereas "onwed" stores only unnamed ones), and it's ambiguous which vector's
  // index the erase() operation is meant.
};

//------------------------------------------------------------------------------

typedef std::function<double (double)> Fct;

class Shape  {        // deals with color and style, and holds sequence of lines 
public:
    void draw() const;                 	// deal with color and draw lines
    virtual void move(int dx, int dy); 	// move the shape +=dx and +=dy

    void set_color(Color col); 
    Color color() const;

    void set_style(Line_style sty);
    Line_style style() const; 

    void set_fill_color(Color col); 
    Color fill_color() const; 

    Point point(int i) const;  		// read only access to points
    int number_of_points() const; 

    Shape(const Shape&) = delete;      	// prevent copying
    Shape& operator=(const Shape&) = delete;

    virtual ~Shape() { }
protected:
    Shape() {};    
    Shape(initializer_list<Point> lst);

    virtual void draw_lines() const;   // draw the appropriate lines
    void add(Point p);                 // add p to points
    void set_point(int i,Point p);     // points[i]=p;
  void clear_points(){points.clear();} // written for Ex 7 of chapter 16
private:
    vector<Point> points;              // not used by all shapes
    Color lcolor {static_cast<int>(fl_color())};// color for lines and characters (with default)
    Line_style ls {0}; 
    Color fcolor {Color::invisible};   // fill color
};

//------------------------------------------------------------------------------

struct Function : Shape {
    // the function parameters are not stored
    Function(Fct f, double r1, double r2, Point orig,
        int count = 100, double xscale = 25, double yscale = 25);
  
  /* commented out since this can cause ambiguity of calling constructors
   */
    Function(double (*f)(double), double r1, double r2, Point orig,
    	int count = 100, double xscale = 25, double yscale = 25);
	// what is (*f)(double)?
	// -> it seems "f" is the variable name. So *f means the pointer to double.
	//    And this way of declaration is called a "function pointer"
	//    https://www.cprogramming.com/tutorial/function-pointers.html
	//    It stores the address of the passed function definition in the main
	//    memory, and we can call the pointed function through the function
	//    pointer.
	//    In this case, double (*f)(double) means a function pointer of functions
	//    that returns a double and takes a double argument.
  //*/
};

  // Ex 2 of Chapter 15
  template<typename T>
  struct Func : Shape {
    Func(Fct f, double r1, double r2, Point orig,
	 int count = 100, double xscale = 25, double yscale = 25, T shift=0.0);

    Func(double (*f)(double), double r1, double r2, Point orig,
	 int count = 100, double xscale = 25, double yscale = 25, T shift=0.0);

    void reset();		// delete points stored in pv
    void set_func(Fct f, double r1=-10, double r2=10, Point orig=Point{200, 200},
		  int count=100, double xscale=25, double yscale=25, T shift=0.0);
    // after reset, set a new function and settings
    // Variables cannot be used in default arguments, like r1=def_r1, because the default
    // values must be known at compile time. Even if we make these default variables
    // as const variables, they are not instantiated until runtime, thus we can't do that.
    // For the same reason, member variables cannot be constexpr, since they are not
    // initialized at compile time, but runtime.
    // Thus there seems no way I can use variables to default arguments, so I have to
    // use numbers to the default arguments above.
    
  private:
    // store constructor arguments
    Fct func;
    double xrange1, xrange2;
    Point origin;
    int num_points;
    double xscale, yscale;
    vector<Point> pv;
    T shift;

    // default argument values to check if other variables are set in set_func()
    const double def_r1{-10};
    const double def_r2{10};
    const Point def_orig{Point{200, 200}};
    const int def_count{100};
    const double def_xscale{25};
    const double def_yscale{25};
    const int def_shift{0};
    // These values are the same as the ones set in set_func().
    // For the reason mentioned below set_func() declaration above, we cannot use
    // variables to default arguments.
    // (constexpr cannot be used for member variables, since a class won't be
    // instantiated at compile time).

    void draw_lines() const override;
    // I think in Window class, draw() function is called (Shape::draw() is public).
    // Then, inside Shape::draw(), draw_lines() is called. Since in Shape::draw(),
    // no qualification before the draw_lines() is attached, that is translated as
    // this->draw_lines(). And since draw_lines() is a virtual function and overriden
    // here (I think in Window class, all classes derived from Shape class is referred
    // by Shape pointer or reference, hence draw_lines() being virtual is necessary
    // I guess?), this overriding Func::draw_lines() is called (although this is private,
    // since Shape::draw() is regarded as a Func's public member, it can call a private
    // function of Func class)
    
    void add_points();		// add points to pv based on the private variables
  };

//------------------------------------------------------------------------------

struct Line : Shape {            // a Line is a Shape defined by two Points
    Line(Point p1, Point p2);    // construct a line from two points

  // this Line::draw_lines() is added at Ex 16 of Ch14, since I wanted to call
  // draw_lines() of Line object from outside of the Line class (in
  // Binary_tree::draw_lines())
  void draw_lines() const{
    Shape::draw_lines();
  }
};

//------------------------------------------------------------------------------

struct Rectangle : Shape {
    Rectangle(Point xy, int ww, int hh);
  // xy: top left corner
    Rectangle(Point x, Point y);
    void draw_lines() const;

    int height() const { return h; }
    int width() const { return w; }
private:
    int h;    // height
    int w;    // width
};

//------------------------------------------------------------------------------

struct Open_polyline : Shape {         // open sequence of lines
    Open_polyline() {}
    Open_polyline(initializer_list<Point> lst): Shape{lst} {}
    
    void add(Point p) { Shape::add(p); }
};

//------------------------------------------------------------------------------

struct Closed_polyline : Open_polyline { // closed sequence of lines
    using Open_polyline::Open_polyline;	 // use Open_polyline's 
					 // constructors (A.16)
    void draw_lines() const;		
};

//------------------------------------------------------------------------------

struct Polygon : Closed_polyline {    // closed sequence of non-intersecting lines
    Polygon() {};
    Polygon(initializer_list<Point> lst);

    void add(Point p);
};

//------------------------------------------------------------------------------

struct Lines : Shape {                 // related lines
    Lines() {}			       // empty
    Lines(initializer_list<pair<Point,Point>> lst); // initialize from a list

    void draw_lines() const;
    void add(Point p1, Point p2);      // add a line defined by two points
};

//------------------------------------------------------------------------------

  // virtual inheritance keyword is added since I make Texted_circle inherit
  // 2 base classes, Text and Circle, to inherit member functions of both base
  // classes (I don't want to re-define functions like set_font(Font) inside
  // Text_circle)
  // This is to avoid duplicate copies of Shape members when instantiating
  // Text_circle class
  // https://www.geeksforgeeks.org/multiple-inheritance-in-c/
struct Text : virtual Shape {
    // the point is the bottom left of the first letter
    Text(Point x, const string& s) 
	: lab{s} 
	{ add(x); }
  Text(){}			// default constructor

    void draw_lines() const;

    void set_label(const string& s) { lab = s; }
    string label() const { return lab; }

    void set_font(Font f) { fnt = f; }
    Font font() const { return fnt; }

    void set_font_size(int s) { fnt_sz = s; }
    int font_size() const { return fnt_sz; }
private:
  string lab{};    // label
    Font fnt{fl_font()};
    int fnt_sz{fl_size()<14?14:fl_size()};
};

//------------------------------------------------------------------------------

struct Axis : Shape {
    enum Orientation { x, y, z };
    Axis(Orientation d, Point xy, int length,
        int number_of_notches=0, string label = "");

    void draw_lines() const override;
    void move(int dx, int dy) override;
    void set_color(Color c);

    Text label;
    Lines notches;
};


  // Ex 6 of Chapter 15
  struct Bar_graph : Shape{
    Bar_graph(Point start, int bar_width, int bar_interval, int yscale=25,
	      const vector<double> &val_vec=vector<double>(),
	      string cap="", const vector<string> &sv=vector<string>());
    // start: bottom left corner of the 1st bar

    void set_fill_color(Color c); // set all Rectangles' fill color to c
    void set_color(Color c); // set all Rectangles' line color to c

    // void erase(int i){		// erase i-th bar
    //   bars.erase(bars.begin()+i);
    // }
    // Since bars is in Vector_ref class, not vector, and since Vector_ref doesn't have
    // erase() member function, it's impossible to do bars.erase().
    // I also thought I would implement erase() member function in Vector_ref, but since
    // Vector_ref class contains 2 vectors, v and owned, and they can have different numbers
    // of elements, it can be ambiguous by index i, which vector's index it means.
    // Thus I quit adding the member function erase() to Vector_ref.

    void add_bars(const vector<double> &vv);		// add Rectangles
    
    Vector_ref<Rectangle> bars;	// bars to be displayed
    // I make this public to enable users to change individual Rectangle's color or
    // fill color by Bar_graph::bars[2].set_fill_color(Color::blue);

    void add_labels(const vector<string> &sv);
    Text caption;
    Vector_ref<Text> labels;	// labels of each bar
    // left public so that users can manipulate them later
    void set_label_color(Color);
    void set_label_font(Font);
    void set_label_font_size(int);
    
    void clear();		// empty labels and bars
  private:
    Point start;
    int bar_width;
    int bar_interval;
    int yscale;
    vector<double> val_vec;

    void draw_lines() const;
  };
  
//------------------------------------------------------------------------------

  // Ex 14 (ch 14)
  // virtual inheritance keyword is added since I make Texted_circle inherit
  // 2 base classes, Text and Circle, to inherit member functions of both base
  // classes (I don't want to re-define functions like set_font(Font) inside
  // Text_circle)
  // This is to avoid duplicate copies of Shape members when instantiating
  // Text_circle class
  // https://www.geeksforgeeks.org/multiple-inheritance-in-c/
struct Circle : virtual Shape {
    Circle(Point p, int rr);    // center and radius

    void draw_lines() const;

    Point center() const ; 
    int radius() const { return r; }
    void set_radius(int rr) 
    { 
	set_point(0,Point{center().x-rr, center().y-rr}); // maintain
							  // the center
	r=rr; 
    }
  
  // Ex 14 of Chapter 14
  virtual void set_label(const string& s){cout << "Circle::set_label() is called\n";}
  // define a function set_label(const string&), which is the same type and name
  // as that in Text class, to use it in Binary_tree::add_text2node().
  // I want to use Vector_ref<Circle>
  // <- What I want to do is to override Circle::set_label() with
  // Text::set_label(). But in Texted_circle, Circle and Text are both base
  // classes of Texted_circle, so I think parallely aligned base classes won't
  // override other base class's member functions
private:
    int r;
};

//------------------------------------------------------------------------------

struct Ellipse : Shape {
    Ellipse(Point p, int w, int h);    // center, min, and max distance from center
  Ellipse(){}			       // default constructor for exercise 1
    void draw_lines() const;

    Point center() const;
    Point focus1() const;
    Point focus2() const;

    void set_major(int ww) 
    { 	
	set_point(0,Point{center().x-ww,center().y-h});	// maintain
							// the center
	w=ww; 
    }
    int major() const { return w; }

    void set_minor(int hh) 
    { 
	set_point(0,Point{center().x-w,center().y-hh});	// maintain
							// the center
	h=hh; 
    }
    int minor() const { return h; }
private:
    int w;
    int h;
};

//------------------------------------------------------------------------------

struct Marked_polyline : Open_polyline {
    Marked_polyline(const string& m) :mark{m} { if(m=="") mark="*"; }
    Marked_polyline(const string& m, initializer_list<Point> lst);
    void draw_lines() const;
private:
    string mark;
};

//------------------------------------------------------------------------------

struct Marks : Marked_polyline {
    Marks(const string& m) :Marked_polyline(m)
    {
        set_color(Color{Color::invisible});
    }
    Marks(const string& m, initializer_list<Point> lst)
	: Marked_polyline{m,lst}
    {
	set_color(Color{Color::invisible});
    }
};

//------------------------------------------------------------------------------

struct Mark : Marks {
    Mark(Point xy, char c) : Marks(string(1,c))
    {
        add(xy);
    }
};

//------------------------------------------------------------------------------

enum class Suffix {
		   none, jpg, gif, png
};

//------------------------------------------------------------------------------

struct Image : Shape {
    Image(Point xy, string file_name, Suffix e = Suffix::none);
    ~Image() { delete p; }
    void draw_lines() const;
    void set_mask(Point xy, int ww, int hh) { w=ww; h=hh; cx=xy.x; cy=xy.y; }

  // for Ex 3 of Chapter 16
  Fl_Image* get_Fl_image_pt(){return p;}
  // I don't know the reason, but when I use *p (and Fl_Image as return type) instead,
  // that caused an error saying it used a private constructor of Fl_Image
  // <- But when I use Fl_Image& as the return type, it compiled OK. I don't know why.

  void resize(int w, int h);	// for Exercise 7 of chapter 16
  void position(Point p){clear_points(); add(p);}
  // for Exercise 7 of chapter 16, for positioning the Image relative to Window,
  // instead of being relative to its previous position like move()
  
  // int test(){return *i;}
  // int *i;
  // it compiles OK. So I don't know why only the above get_Fl_image() got an error
private:
    int w,h;  // define "masking box" within image relative to position (cx,cy)
    int cx,cy; 
    Fl_Image* p;
    Text fn;
};

//------------------------------------------------------------------------------

struct Bad_image : Fl_Image {
    Bad_image(int h, int w) : Fl_Image(h,w,0) { }
    void draw(int x,int y, int, int, int, int) { draw_empty(x,y); }
};

//------------------------------------------------------------------------------




// exercise 1
  // There are 3 inheritance modes:
  // https://www.tutorialspoint.com/cplusplus/cpp_inheritance.htm
  // If no inheritance keyword is used, the default inheritance mode is used.
  // The default inheritance differs in struct and class. struct's default inheritance is
  // public, whereas class's default is private
struct Arc : Ellipse {
  // set Ellipse as Arc's base class.
  // inherits Ellipse's member functions, variables, but doesn't inherit its
  // constructors and destructors (this is how the inheritance is defined in C++)
  // To inherit the base class's constructors as well, use the following statement:
  // using Ellipse::Ellipse;
  // This time I prepare a constructor for this class
  Arc(Point p, int ww, int hh, int ssd, int eed)
    : w{ww}, h{hh}, sd{ssd}, ed{eed}, Ellipse{p, ww, hh}
  {add(Point{p.x-w,p.y-h});}
  // store the point as the top left corner of the smallest rectangle which
  // surrounds the ellipse
  // Ellipse{p, ww, hh} is needed, because when creating an object of a derived
  // class, we first need to initialize the base class, though in this case,
  // Ellipse's w and h cannot be accessed (cannot be used from Arc class)
  // https://stackoverflow.com/questions/23647409/
  // Or other ways are: to make a default constructor for Ellipse, or to make Arc
  // inherit from Shape class, not Ellipse class

  // for exercise 2 (class Box), define an empty constructor and update function
  Arc(){}
  void update_arc(Point p, int ww, int hh, int ssd, int eed);
  
  // override the draw_lines() function of the base class (Ellipse)
  void draw_lines() const;

private:			// add 2 new member variables
  int sd;			// start degree
  int ed;			// end degree
  // Although a derived class (child class) inherits the structures of the base
  // class, private members of a base class cannot be accessed from even the
  // derived class.
  // So to use variables w and h, I need to declare them again in this Arc class
  int w;
  int h;
};


  // exercise 2
  // use Arc and Lines classes declared above
  // Make it inherit from Shape, because Simple_window's attach() function needs
  // a reference to Shape object attach(Shape& s) (to make Box class recognized
  // as a Shape class as well)
  // exercise 6
  // for adding a text in the box, I make Box derived from Text class
  // (since when using struct, the default inheritance mode is public, I can
  // use Text's public members as public and private members as private in
  // Box class)
  struct Box : Text {
    Box(Point p, int ww, int hh);
    Box(Point p, int ww, int hh, const string& s);
    void draw_lines() const;
  private:
    Arc a1, a2, a3, a4;		// Arcs for the 4 corners
    Lines ls;
  };


  // exercise 3
  struct Arrow : Shape{
    Arrow(Point p1, Point p2);	// inspired from Line class

    void draw_lines() const;
    /*
      error point. Somehow, if I don't attach "const" argument in this 
      draw_lines() function, that Arrow::draw_lines() is not used when attached
      to a window, instead, Shape::draw_lines() is used.
      I don't know why, but I think this behavior is related to how the
      draw_lines() function is called when the object is attached to a window
     */

    void set_color(Color col);
    void set_fill_color(Color fcol);
    // for these 2 functions, overwrite those of Shpae class, bacause in this
    // class, arrow_head is involved
    
  private:
    Closed_polyline arrow_head;
  };


  // exercise 10 of Chapter 13
  struct Regular_polygon : Polygon{
    // if the base class is Polygon, what's inherited is only the function
    // Polygon::add(Point) (since constructors are not inherited)
    // This Polygon::add(Point) checks if there is no intersection between
    // the lines. Other than that, it is the same as Closed_polyline::add()
    // (check the code in Graph.cpp)

    Regular_polygon(Point center, int num_sides, double dist);
  };


  // exercise 18
  struct Poly : Polygon{
    Poly(initializer_list<Point> lst);
    // initializer_list: p447
    // As commented above in Regular_polygon, by inheriting from Polygon, Poly
    // inherits its public members, including Polygon::add(), which checks if
    // there is any intersection in the lines

    // Polygon inherits Closed_polyline's draw_lines() as a public member,
    // and Poly inherits that draw_lines() as a public member
  };


  // exercise 19
  struct Star : Polygon{
    Star(Point center, int num_points, double dist);
    // dist: distance from the center point to the farthest point of the star
  };


  // Chapter 14
  // Ex1
  struct Smily : Circle {
    void draw_lines() const;

    Smily(Point p, int rr);	// center and radius
  };

  struct Frowny : Circle {
    void draw_lines() const;
    Frowny(Point, int);		// center and radius
  };

  //Ex3
  struct abst {
    int i;
  protected:
    abst(){}			// protect any constructor
  };

  // Ex4
  struct Immobile_Circle : Circle {
    using Circle::Circle;	// use constructors of Circle
    void move() = delete;
    // delete move() inherited from Shape class
    // Note: void move(int, int) = delete; causes an error as follows:
    // error: deleted function 'move' cannot override a non-deleted function
    // So I guess to delete (disable) a member function of a base class,
    // we need to omit the argument type.
    // Another way to disable part of a base class is to change its
    // access, as follows:
    // private:
    //   using Shape::move;
    // In this way, we need to omit the parenthesis.
    // For details, see
    // https://www.learncpp.com/cpp-tutorial/hiding-inherited-functionality/
  };

  // Ex5
  struct Striped_rectangle : Rectangle {
    //using Rectangle::Rectangle;	// use Rectangle's constructors
    // At the constructor, I want to store the stripe lines, so I decide
    // to let this class have its own constructors
    Striped_rectangle(Point xy, int ww, int hh);
    Striped_rectangle(Point x, Point y);
    void draw_lines() const;

  private:
    Lines ls;			// lines for the stripe
  };

  // Ex 6
  struct Striped_circle : Circle {
    Striped_circle(Point p, int rr);
    void draw_lines() const;
  private:
    Lines ls;
  };

  // Ex 7
  struct Striped_closed_polyline : Closed_polyline {
    using Closed_polyline::Closed_polyline;
    // Unlike Ex 5, at the time of using a constructor, we cannot decide
    // the shape of Closed_polyline, thus cannot store Lines object.
    // So this time, I use Closed_polyline's constructors
    void draw_lines() const;
    // in this draw_lines(), I store all Lines
    void finish();
    // tell that adding points are finished, so that we can start adding
    // stripe lines to ls
    // (At first, I tried to do that in draw_lines(), but since this
    // function is const function, I cannot make any change to the object
    // state in the function)
  private:
    Lines ls;
  };


  // Ex 9
  struct Group : Shape {
    void draw_lines() const override;
    void add(Shape& s){vrs.push_back(s);}
    // when an actual object is passed (add(r))
    void add(Shape* s){vrs.push_back(s);} // when an unnamed object is passed
    void move(int, int) override;	// override Shape::move()
    void move_ith(int, int, int);
    // for convenience, add another move function which moves only the ith
    // Shape stored in the vector vrs

    // Note: It seems "override" specifier cannot be put in the definition, but
    // it must be in declarations.

    Group(){}

    // set color/fill color of each of the stored Shape objects in vrs
    void set_color(Color);
    void set_fill_color(Color);
    void stored_fill_color();	// for debug

    // set color/fill color of stored objects in the specified range
    void set_color_with_range(int, int, Color);
    // arguments: (beginning of the index, end of the index, Color)
    void set_fill_color_with_range(int, int, Color);

    // Notice: although we can store various kinds of Shape objects in vrs, we
    // cannot call member functions of these objects which are not defined in
    // Shape class, because at compile time, it is regarded as Shape object
    // even if its Shape pointer is pointing, say, Circle

    Shape& operator[](int i){return vrs[i];}
    const Shape& operator[](int i) const {return vrs[i];}
    // This const Shape& operator[] is needed when a Group object is defined
    // as const, and the user tries to use this operator Group[i]

    int size(){return vrs.size();}
  private:
    Vector_ref<Shape> vrs;
    // this is essentially similar, or the same as vector<Shape*> (vector of
    // pointers to Shape class objects. See the definition of Vector_ref
    // template)
  };


  // Ch 14 Ex 14
  struct Texted_circle : Circle, Text {
    // Be careful!! The order of listing the base classes matter!
    // When Text comes before Circle, in the Text_circle constructors,
    // in its member initializers, regardless of its order (even if
    // : Circle(c, rr), Text(Point{c.x, c.y}, "")), Text is first initialized,
    // hence point(0) stores Text's point Point{c.x, c.y}, not top left corner
    // of the circle (Point{c.x-r,c.y-r}). And since Center::center() uses
    // point(0), if point(0) stores Text's point, it causes an erronious
    // positioning of the nodes
    
    //using Circle::Circle;	// use Circle's constructors
    // <- if doing so, when Texted_circle{Point, int} is called, we cannot
    // add the 2nd point (stored as point(1)) which is used to draw the text
    Texted_circle(Point c, int rr);
    Texted_circle(Point c, int rr, string s);
    void draw_lines() const;

    void set_label(const string& s) { Text::set_label(s); }
    // To use Vector_ref<Circle> instead of Vector_ref<Texted_circle> for Ex 16,
    // I added  virtual void set_label(const string& s) in Circle class, which
    // does nothing, but which enables using circle_vec[ni].set_label(s) in
    // Binary_tree::add_text2node(). And I want to call Text::set_label() when 
    // calling (Texted_circle object).set_label(string), so I override
    // Circle::set_label(s) with this set_label(), which in turn calls
    // Text::set_label(s)

    void set_font_color(Color cc){font_color = cc;}
    // private members of this class is inherited from Circle, Text, and Shape's
    // private members

  private:
    Color font_color;		// added for ex 14 of ch19
  };


  // Ch14 Ex11, Ex 13, Ex 14
  struct Binary_tree : Shape {
    void draw_lines() const;
    Binary_tree(Point root_pt, int lvl);
    // Point of the root coordinates, and the level of this binary tree
    Binary_tree(Point root_pt, int lvl, char c);
    // the last argument is either 'u', meaning up-arrow, or 'd', meaning down-
    // arrow. So if the 3rd argument is passed in the constructor, it
    // automatically switches from Lines to Arrow.
    
    void set_arrow_color(Color);
    void set_node_color(Color);
    void add_text2node(const string& s, const string& t); // Ex 14
  protected:
    virtual void draw_nodes() const;
    Vector_ref<Circle> circle_vec;
    //Vector_ref<Texted_circle> circle_vec; // Ex 14
    //Lines lines;
    Vector_ref<Line> line_vec;	// to enable deleting all Line elements, I change
    // to store them in Vector_ref<Line>
    Vector_ref<Arrow> arrow_vec;
    int node_radius;
    // to make these variables accessible from derived classes as well, I put
    // them in the protected region
    int level;

    void push_back_unnamed_circle_obj(Vector_ref<Circle>& cir_vec,
				      Point center,
				      char circle_type);
    void _store_nodes(Point root_pt, int lvl, char circle_type);
    void _store_lines(int lvl, char c);
    // _store_lines() assumes node Circles are already stored
    // To use it in Ex 16, I moved these functions from private to protected

    Binary_tree(){};
    // empty constructor, for the use in derived classes from Binary_tree
  private:
    //Group bintree_gp;
    // since Binary_tree is a collection of different kinds of objects (Lines,
    // Circle), I store them internally in Group object that I defined in Ex 9.
    // I used Group instead of directly using Vector_ref, because Group class
    // has some useful member functions such as the one changing all the stored
    // objects' color or fill_color
    // <- Since it is easier to store Circles and Lines separately (because
    // by doing so, I can refer to a parent node by index i/2 where i is the
    // index of the current node)
    double _get_intersec_x(double a, double b, const Circle &cir, char c);
    // get the x value of an intersection of a line y=ax+b and a Circle. 
    // There are 2 intersections between a line and a circle, and char c
    // specifies which of the 2 x values is returned: '+' returns larger one,
    // and '-' returns the smaller one.
    // Since this function is used only for this class, not intended to be used
    // for derived classes, I keep this function private    
  };


  // Ch14 Ex 12
  struct Binary_tree_tri : Binary_tree {
    Binary_tree_tri(Point root_pt, int lvl);
    //using Binary_tree::Binary_tree;
    // try if I can use constructors of Binary_tree. But since in this
    // derived class, we cannot use _get_intersec_x(), which is used in
    // Binary_tree's constructor and private for Binary_tree, I guess this
    // would fail.
    // <- since I want to store Regular_polygon objects in the constructor,
    // I decided to define a new constructor of this class's own

    //void draw_lines() const;
    // <- I don't need to override Binary_tree::draw_lines()
  protected:
    void draw_nodes() const;
    Vector_ref<Regular_polygon> reg_poly_vec;
    // vector of Regular_polygon, which I defined in exercise 10 of Chapter 13.
    
  };


  // Ch 14 Ex 16
  struct Controller {
    virtual void on(){is_on = true;}
    virtual void off(){is_on = false;}
    virtual void set_level(int l){level = l;}
    virtual void show() const = 0;
  protected:
    bool is_on{false};
    int level{0};
  };

  struct Controller_test : Controller{
    void show() const {
      cout << "This controller is " << (is_on? "on" : "off") << endl;
      cout << "The level of this controller is " << level << endl;
    }
  };

  struct Shape_controller : Controller {
    // set the visibility of the shape object to visible
    void on(){
      Color c{shape_ptr->color()};
      c.set_visibility(Color::visible);
      shape_ptr->set_color(c);
    }
    // set the visibility of the shape object to invisible
    void off(){
      Color c{shape_ptr->color()};
      c.set_visibility(Color::invisible);
      shape_ptr->set_color(c);
    }
    // change the line style (only line width)
    void set_level(int lv){
      if(lv < 0)
	error("Error in Shape_controller::set_level(). The argument must be 0 or greater int.");
      Line_style ls{shape_ptr->style()};
      shape_ptr->set_style(Line_style{static_cast<Line_style::Line_style_type>(ls.style()), lv});
    }
    // show if the pointed Shape object is visible or not
    void show() const {
      cout << "The pointed Shape object's line color is set to be "
	   << (shape_ptr->color().visibility()== static_cast<char>(Color::visible) ? "visible" :
	       "invisible") << endl;
    }
    void set_shape(Shape& s){shape_ptr = &s;}
    // if I make the argument const Shape& s, I couldn't get its address with
    // this way (&s), so I removed its const keyword (now it works).
    // I don't know why this const causes such an error. Getting its address
    // doesn't change its value, so it should be ok, shouldn't it??
  private:
    Shape* shape_ptr;
  };

  
  // Binary tree class that can switch its node among normal Circle, Smily, and
  // Frowny (Smily and Frowny are from Ex 1 of Ch14) from public member
  struct Switch_nodes_Binary_tree : Binary_tree {
    void add_text2node(const string &, const string&) = delete;
    // Texted_circle is no longer used in this class, so delete this member func

    //using Binary_tree::Binary_tree; // use Binray_tree's constructors

    Switch_nodes_Binary_tree(Point root_pt, int lvl);
    Switch_nodes_Binary_tree(Point root_pt, int lvl, char al);
    Switch_nodes_Binary_tree(Point root_pt, int lvl, char al, char circle_type);
    // additional constructor

    void switch_node_type(char circle_type);
    // delete existing Circle objects stored in circle_vec, and store Circles of
    // type circle_type ('c/t/s/f' for Circle, Texted_circle, Smily, and Frowny
    // respectively)

    char node_type(){return circle_type;}
    int tree_level(){return level;}
    void recreate_bin_tree(int lvl); // recreate binary tree with the given level
    // without chaning node type and Line type (Line/Arrow)
  private:
    char circle_type;
    // 'c/s/f/t' for Circle, Smily, Frowny, and Texted_circle
    char line_type;
    // 'l/u/d' for Line, upper-Arrow, down-Arrow
  };
  
  // Switches the nodes of Binary_tree object
  // on() turns the nodes into Smily, off() turns the nodes into normal Circle
  struct Bin_tree_nodes_controller : Controller {
    void on(){
      if(snbt_ptr->node_type() != 's')
	snbt_ptr->switch_node_type('s');
    }
    void off(){
      if(snbt_ptr->node_type() != 'c')
	snbt_ptr->switch_node_type('c');
    }

    // re-create the Binary_tree with the given level
    void set_level(int l){snbt_ptr->recreate_bin_tree(l);}

    // show() prints out current node type and current level
    void show() const {
      cout << "Node type: ";
      switch(snbt_ptr->node_type()){
      case 'c':
	cout << "Circle\n";
	break;
      case 's':
	cout << "Smily\n";
	break;
      case 't':
	cout << "Texted_circle\n";
	break;
      case 'f':
	cout << "Frowny\n";
	break;
      default:
	cout << "Unknown, most probably due to some error before\n";
	break;
      }
      
      cout << "Switch_nodes_Binary_tree level: " << snbt_ptr->tree_level()
	   << endl;
    }
    
    void set_switch_node_bin_tree(Switch_nodes_Binary_tree& s){snbt_ptr = &s;}
  private:
    Switch_nodes_Binary_tree* snbt_ptr;
  };
  

  // Ex 10 of Chapter 15
  struct Scatter_graph : Shape {
    Scatter_graph(Point origin, const vector<pair<double, double>> pv, 
		  double xscale=20, double yscale=20,
		  const vector<double> &vv=vector<double>(), // assigned values for each
		  const vector<string> &sv=vector<string>()); // for labels of each point
    // pv : pairs of 2 values, like (12.3, 3.12)
    // vv : values assigned to each point (pair). The radius of the point of a pair becomes
    //      larger proportionally to the value the point has

    void draw_lines() const;

    void set_fill_color(Color c){
      for(int i=0; i<scpoints.size(); ++i)
	scpoints[i].set_fill_color(c);
    }	    
    void set_color(Color c){
      for(int i=0; i<scpoints.size(); ++i)
	scpoints[i].set_color(c);
    }
    Vector_ref<Texted_circle> scpoints;
    // made public to let users manipulate each circle, such as adding a label
  };
  
} // of namespace Graph_lib

#endif
