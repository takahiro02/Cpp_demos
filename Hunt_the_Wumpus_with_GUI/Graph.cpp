
//
// This is a GUI support code to the chapters 12-16 of the book
// "Programming -- Principles and Practice Using C++" by Bjarne Stroustrup
//

#include <FL/Fl_GIF_Image.H>
#include <FL/Fl_JPEG_Image.H>
#include <FL/Fl_PNG_Image.H>	// Ex 7 of chapter 16
#include "Graph.h"

#include <iostream>

//------------------------------------------------------------------------------

namespace Graph_lib {

//------------------------------------------------------------------------------

Shape::Shape(initializer_list<Point> lst)
{
	for(Point p : lst) add(p);
}

//------------------------------------------------------------------------------

void Shape::set_color(Color col)
{
	lcolor = col;
}

//------------------------------------------------------------------------------

Color Shape::color() const
{
 	return lcolor; 
}

//------------------------------------------------------------------------------

void Shape::set_style(Line_style sty)
{ 
	ls = sty; 
}

//------------------------------------------------------------------------------

Line_style Shape::style() const 
{ 
	return ls; 
}

//------------------------------------------------------------------------------

void Shape::set_fill_color(Color col)
{
	fcolor = col;
	/*
	  Since fcolor is Color class, it has 2 private member variables,
	  v (char, used for visibility) and c (Fl_Color).
	  fcolor's default visibility is invisible (by Shape class's 
	  initialization). But since col is also a Color class object, it also
	  has its own visibility. And when a Color object is initialized with
	  only the Color_type, its visibility is automatically set to visible
	  (from Color's constructor). Thus when col is specified with, say,
	  Color::red, its visibility is visible.
	  Thus when fcolor = col, and say, col = Color::red, fcol's visibility 
	  turns visible
	  Also, from the usage of set_color() and set_fill_color(), it seems
	  that Color::red is translated as Color{Color::red}
	 */
}

//------------------------------------------------------------------------------

Color Shape::fill_color() const 
{
	return fcolor;
}

//------------------------------------------------------------------------------

void Shape::add(Point p)     // protected
{
    points.push_back(p);
}

//------------------------------------------------------------------------------

Point Shape::point(int i) const 
{ 
	return points[i]; 
}

//------------------------------------------------------------------------------

int Shape::number_of_points() const 
{ 
	return int(points.size()); 
}

//------------------------------------------------------------------------------

void Shape::set_point(int i,Point p)        // not used; not necessary so far
{
    points[i] = p;
}

//------------------------------------------------------------------------------
void Shape::draw_lines() const
{
    if (fill_color().visibility()) {
        fl_color(fill_color().as_int());
        fl_begin_complex_polygon();
        for(int i=0; i<number_of_points(); ++i){
            fl_vertex(point(i).x, point(i).y);
        }
        fl_end_complex_polygon();
        fl_color(color().as_int());    // reset color
    }

    if (color().visibility() && 1<points.size())    // draw sole pixel?
        for (unsigned int i=1; i<points.size(); ++i)
            fl_line(points[i-1].x,points[i-1].y,points[i].x,points[i].y);
}

//------------------------------------------------------------------------------

void Shape::draw() const
{
  //cout << "Shape::draw() is called\n";
    Fl_Color oldc = fl_color();
    // there is no good portable way of retrieving the current style
    fl_color(lcolor.as_int());            // set color
    fl_line_style(ls.style(),ls.width()); // set style
    draw_lines();
    // This is implicitly translated as this->draw_lines(), where this is a
    // pointer to the object which calls draw()
    fl_color(oldc);      // reset color (to previous)
    fl_line_style(0);    // reset line style to default
}

//------------------------------------------------------------------------------


void Shape::move(int dx, int dy)    // move the shape +=dx and +=dy
{
    for (int i = 0; i<points.size(); ++i) {
        points[i].x+=dx;
        points[i].y+=dy;
    }
}

//------------------------------------------------------------------------------

Line::Line(Point p1, Point p2)    // construct a line from two points
{
    add(p1);    // add p1 to this shape
    add(p2);    // add p2 to this shape
}

//------------------------------------------------------------------------------

void Lines::add(Point p1, Point p2)
{
    Shape::add(p1);
    Shape::add(p2);
}

//------------------------------------------------------------------------------

// draw lines connecting pairs of points
void Lines::draw_lines() const
{
    if (color().visibility())
        for (int i=1; i<number_of_points(); i+=2)
            fl_line(point(i-1).x,point(i-1).y,point(i).x,point(i).y);
}

//------------------------------------------------------------------------------

Lines::Lines(initializer_list<pair<Point,Point>> lst)
{
	for(auto p: lst) add(p.first, p.second);
}

//------------------------------------------------------------------------------

// does two lines (p1,p2) and (p3,p4) intersect?
// if so, return the distance of the intersect point as distances from p1
inline pair<double,double> line_intersect(Point p1, Point p2, Point p3, Point p4, bool& parallel) 
{
    double x1 = p1.x;
    double x2 = p2.x;
    double x3 = p3.x;
    double x4 = p4.x;
    double y1 = p1.y;
    double y2 = p2.y;
    double y3 = p3.y;
    double y4 = p4.y;

    double denom = ((y4 - y3)*(x2-x1) - (x4-x3)*(y2-y1));
    if (denom == 0){
        parallel= true;
        return pair<double,double>(0,0);
    }
    parallel = false;
    return pair<double,double>( ((x4-x3)*(y1-y3) - (y4-y3)*(x1-x3))/denom,
                                ((x2-x1)*(y1-y3) - (y2-y1)*(x1-x3))/denom);
}

//------------------------------------------------------------------------------

//intersection between two line segments
//Returns true if the two segments intersect,
//in which case intersection is set to the point of intersection
bool line_segment_intersect(Point p1, Point p2, Point p3, Point p4, Point& intersection){
   bool parallel;
   pair<double,double> u = line_intersect(p1,p2,p3,p4,parallel);
   if (parallel || u.first < 0 || u.first > 1 || u.second < 0 || u.second > 1) return false;
   intersection.x = p1.x + u.first*(p2.x - p1.x);
   intersection.y = p1.y + u.first*(p2.y - p1.y);
   return true;
}

//------------------------------------------------------------------------------

void Polygon::add(Point p)
{
    int np = number_of_points();

    if (1<np) {    // check that thenew line isn't parallel to the previous one
        if (p==point(np-1)) error("polygon point equal to previous point");
        bool parallel;
        line_intersect(point(np-1),p,point(np-2),point(np-1),parallel);
        if (parallel)
            error("two polygon points lie in a straight line");
    }
    for (int i = 1; i<np-1; ++i) {    // check that new segment doesn't interset and old point
        Point ignore{0,0};
        if (line_segment_intersect(point(np-1),p,point(i-1),point(i),ignore))
            error("intersect in polygon");
    }

    Closed_polyline::add(p);
}

//------------------------------------------------------------------------------

Polygon::Polygon(initializer_list<Point> lst)
{
	for(Point p : lst) add(p);
}

//------------------------------------------------------------------------------

void Closed_polyline::draw_lines() const
{
    Open_polyline::draw_lines();    // first draw the "open poly line part"

    // then draw closing line:
    if (2<number_of_points() && color().visibility())
        fl_line(point(number_of_points()-1).x, 
        point(number_of_points()-1).y,
        point(0).x,
        point(0).y);

}

//------------------------------------------------------------------------------

void draw_mark(Point xy, char c)
{
    static const int dx = 4;
    static const int dy = 4;

    string m(1,c); // string holding a single char c
    fl_draw(m.c_str(),xy.x-dx,xy.y+dy);
}

//------------------------------------------------------------------------------

void Marked_polyline::draw_lines() const
{
    Open_polyline::draw_lines();
    for (int i=0; i<number_of_points(); ++i) 
        draw_mark(point(i),mark[i%mark.size()]);
}

//------------------------------------------------------------------------------

Marked_polyline::Marked_polyline(const string& m, initializer_list<Point> lst)
	: Open_polyline{lst},mark{m}
{
	if(m=="")
		mark = "*";
}

//------------------------------------------------------------------------------

Rectangle::Rectangle(Point xy, int ww, int hh) : w(ww), h(hh)
{
     if (h<=0 || w<=0) 
	error("Bad rectangle: non-positive side");
     add(xy);
}

//------------------------------------------------------------------------------

Rectangle::Rectangle(Point x, Point y) : w(y.x-x.x), h(y.y-x.y)
{
    if (h<=0 || w<=0) 
	error("Bad rectangle: first point is not top left");
    add(x);
}

//------------------------------------------------------------------------------

void Rectangle::draw_lines() const
{
  /*
  cout << "Rectangle::draw_lines() is called\n";
  cout << "fill_color() == " << fill_color().as_int()
       << " (Color::green.as_int() == " << Color{Color::green}.as_int()
								 << ")"<< endl;
  */
  
    if (fill_color().visibility()) {    // fill
        fl_color(fill_color().as_int());
        fl_rectf(point(0).x,point(0).y,w,h);
    }

    if (color().visibility()) {    // lines on top of fill
        fl_color(color().as_int());
        fl_rect(point(0).x,point(0).y,w,h);
    }
}

//------------------------------------------------------------------------------

Circle::Circle(Point p, int rr)    // center and radius
:r(rr)
{
    add(Point{p.x-r,p.y-r});       // store top-left corner
}

//------------------------------------------------------------------------------

Point Circle::center() const
{
    return Point{point(0).x+r, point(0).y+r};
}

//------------------------------------------------------------------------------

void Circle::draw_lines() const
{
  // from https://stackoverflow.com/questions/25549789/
  if (fill_color().visibility()) {    // fill
    fl_color(fill_color().as_int());
    fl_pie(point(0).x,point(0).y,r+r-1,r+r-1,0,360);
    fl_color(color().as_int()); // reset color
  }
  
  if (color().visibility()) {
    fl_color(color().as_int());
    fl_arc(point(0).x,point(0).y,r+r,r+r,0,360);
  }
}

//------------------------------------------------------------------------------

Ellipse::Ellipse(Point p, int w, int h): w{w}, h{h}
{
	add(Point{p.x-w,p.y-h});	
}

//------------------------------------------------------------------------------

Point Ellipse::center() const 
{ 
	return Point{point(0).x+w,point(0).y+h}; 
}

//------------------------------------------------------------------------------

Point Ellipse::focus1() const 
{ 
	if(h<=w) // foci are on the x axis
		return Point{center().x+int(sqrt(double(w*w-h*h))),center().y}; 
	else     // foci are on the y axis
		return Point{center().x,center().y+int(sqrt(double(h*h-w*w)))}; 
}

//------------------------------------------------------------------------------


Point Ellipse::focus2() const
{
        if(h<=w) // foci are on the x axis
                return Point{center().x-int(sqrt(double(w*w-h*h))),center().y};
        else     // foci are on the y axis
                return Point{center().x,center().y-int(sqrt(double(h*h-w*w)))};
}  

//------------------------------------------------------------------------------

void Ellipse::draw_lines() const
{
    if (color().visibility())
        fl_arc(point(0).x,point(0).y,w+w,h+h,0,360);
}

//------------------------------------------------------------------------------

void Text::draw_lines() const
{
    	int ofnt = fl_font();
	int osz = fl_size();
	fl_font(fnt.as_int(),fnt_sz);
	fl_draw(lab.c_str(),point(0).x,point(0).y);
	fl_font(ofnt,osz);
}

//------------------------------------------------------------------------------

Axis::Axis(Orientation d, Point xy, int length, int n, string lab) :
    label(Point{0,0},lab)
{
    if (length<0) error("bad axis length");
    switch (d){
    case Axis::x:
    {
        Shape::add(xy); // axis line
        Shape::add(Point{xy.x+length,xy.y});

        if (1<n) {      // add notches
            int dist = length/n;
            int x = xy.x+dist;
            for (int i = 0; i<n; ++i) {
                notches.add(Point{x,xy.y},Point{x,xy.y-5});
                x += dist;
            }
        }
        // label under the line
        label.move(xy.x+length/3,xy.y+20);
        break;
    }
    case Axis::y:
    {
        Shape::add(xy); // a y-axis goes up
        Shape::add(Point{xy.x,xy.y-length});

        if (1<n) {      // add notches
            int dist = length/n;
            int y = xy.y-dist;
            for (int i = 0; i<n; ++i) {
                notches.add(Point{xy.x,y},Point{xy.x+5,y});
                y -= dist;
            }
        }
        // label at top
        label.move(xy.x-10,xy.y-length-10);
        break;
    }
    case Axis::z:
        error("z axis not implemented");
    }
}

//------------------------------------------------------------------------------

void Axis::draw_lines() const
{
    Shape::draw_lines();
    notches.draw();  // the notches may have a different color from the line
    label.draw();    // the label may have a different color from the line
}

//------------------------------------------------------------------------------

void Axis::set_color(Color c)
{
    Shape::set_color(c);
    notches.set_color(c);
    label.set_color(c);
}

//------------------------------------------------------------------------------

void Axis::move(int dx, int dy)
{
    Shape::move(dx,dy);
    notches.move(dx,dy);
    label.move(dx,dy);
}

//------------------------------------------------------------------------------

// Ex 6 of Chapter 15
// Bar_graph::add_bars() needs to be defined before the constructo, since this is used
// inside the constructor
void Bar_graph::add_bars(const vector<double> &vv){
  int r{static_cast<int>(start.x+(bar_width+bar_interval)*bars.size())};
  // if already some bars are stored, start from the right next to the last bar
  for(int i=0; i<vv.size(); ++i){
    bars.push_back(new Rectangle{Point{r, start.y-int(vv[i]*yscale)}, bar_width,
    									     int(vv[i]*yscale)});
    // paint the newly added bar with the fill color and color of this Bar_graph object
    bars[bars.size()-1].set_fill_color(fill_color());
    bars[bars.size()-1].set_color(color());
    
    r += bar_width + bar_interval;
  }

  // store them after adding Rectangles, since if they are added before it, "r" thinks
  // we already stores some elements of val_vec even if I meant it to be empty
  for(int i=0; i<vv.size(); ++i){
    val_vec.push_back(vv[i]);
  }
}

// this structure is copied from Bar_graph::add_bars()
void Bar_graph::add_labels(const vector<string> &sv){
  int r{static_cast<int>(start.x+(bar_width+bar_interval)*labels.size())};
  // if already some labels are stored, start from the right next to the last bar
  for(int i=0; i<sv.size(); ++i){
    labels.push_back(new Text{Point{r, start.y+15}, sv[i]});  
    r += bar_width + bar_interval;
  }
}

Bar_graph::Bar_graph(Point start, int bar_width, int bar_interval, int yscale,
		     const vector<double> &val_vec,
		     string cap, const vector<string> &sv)
  : start{start}, bar_width{bar_width}, bar_interval{bar_interval}, yscale{yscale},
    // val_vec is added in add_bars()
    caption{Point{0,0}, cap}
// as in Axis::Axis, since we cannot set the point of Text class later, we need to
// initialize Text class with some point (we can move it later with Text::move())
{
  add_bars(val_vec);

  add_labels(sv);

  // move the caption to the center of existing bars for x, and below bars for y
  int x;
  x = (start.x + (start.x+(bar_width+bar_interval)*bars.size())) / 2;
  int caption_size{static_cast<int>(caption.label().size() * caption.font_size())};
  x -=  caption_size / 4;	// to make the caption fit to the center
  // middle of the start point and the last bar
  caption.move(x, start.y+30);
}

void Bar_graph::draw_lines() const {
  for(int i=0; i<bars.size(); ++i)
    bars[i].draw_lines();

  for(int i=0; i<labels.size(); ++i)
    labels[i].draw_lines();
  
  caption.draw_lines();
}

void Bar_graph::set_fill_color(Color c){
  // first, store it to this Bar_graph object itself, so that I can refer to it later
  // in add_bars();
  Shape::set_fill_color(c);
  // Without Shape::, set_fill_color(c) calls Bar_graph::set_fill_color() itself,
  // which causes infinite loop, which causes a segmentation fault.
  // This is because without the qualification, set_fill_color() is regarded as
  // this->set_fill_color(), and since this is Bar_graph type, even when
  // Shape::set_fill_color() is not a virtual function, the compiler calls
  // Bar_graph::set_fill_color
  
  for(int i=0; i<bars.size(); ++i)
    bars[i].set_fill_color(c);
}

void Bar_graph::set_color(Color c){
  Shape::set_color(c);
  
  for(int i=0; i<bars.size(); ++i)
    bars[i].set_color(c);
}

void Bar_graph::set_label_color(Color c){
  for(int i=0; i<labels.size(); ++i)
    labels[i].set_color(c);
}

void Bar_graph::set_label_font(Font f){
  for(int i=0; i<labels.size(); ++i)
    labels[i].set_font(f);
}

void Bar_graph::set_label_font_size(int s){
  for(int i=0; i<labels.size(); ++i)
    labels[i].set_font_size(s);
}

void Bar_graph::clear(){
  bars.clear();
  labels.clear();
}


//------------------------------------------------------------------------------

Function::Function(Fct f, double r1, double r2, Point xy,
                   int count, double xscale, double yscale)
// graph f(x) for x in [r1:r2) using count line segments with (0,0) displayed at xy
// x coordinates are scaled by xscale and y coordinates scaled by yscale
{
    if (r2-r1<=0) error("bad graphing range");
    if (count <=0) error("non-positive graphing count");
    double dist = (r2-r1)/count;
    double r = r1;
    for (int i = 0; i<count; ++i) {
        add(Point{xy.x+int(r*xscale),xy.y-int(f(r)*yscale)});
        r += dist;
    }
}


Function::Function(double (*f)(double), double r1, double r2, Point xy,
		int count, double xscale, double yscale): 
			Function{static_cast<Fct>(f),r1,r2,xy,count,xscale,yscale}
{
  //cout << "Function with function pointer is called\n";
}


// Ex 2 of Chapter 15
template<typename T>
Func<T>::Func(Fct f, double r1, double r2, Point xy,
	   int count, double xscale, double yscale, T shift)
// graph f(x) for x in [r1:r2) using count line segments with (0,0) displayed at xy
// x coordinates are scaled by xscale and y coordinates scaled by yscale
  : func{f}, xrange1{r1}, xrange2{r2}, origin{xy}, num_points{count}, xscale{xscale},
    yscale{yscale}, shift{shift}
{
  add_points();
}

template<typename T>
Func<T>::Func(double (*f)(double), double r1, double r2, Point xy,
	   int count, double xscale, double yscale, T shift): 
  Func{static_cast<Fct>(f),r1,r2,xy,count,xscale,yscale, shift}
{
  //cout << "Function with function pointer is called\n";
}

// Add points to the point vector defined in Func, not Shape class, since Shape class's
// point vector cannot be deleted from outside of Shape. Since we want to reuse the same
// Func object, and store new points, the points must be stored in Func, not Shape
// The contents of this function is from Function class's constructor
// This function adds points based on the private variables stored in the constructor
template<typename T>
void Func<T>::add_points(){
  if (xrange2-xrange1<=0) error("bad graphing range");
    if (num_points <=0) error("non-positive graphing count");
    double dist = (xrange2-xrange1)/num_points;
    double r = xrange1;
    for (int i = 0; i<num_points; ++i) {
        pv.push_back(Point{origin.x+int(r*xscale),origin.y-int(func(r)*yscale+shift)});
        r += dist;
    }
}

template<typename T>
void Func<T>::reset(){
  pv.clear();
}
// Notice: Even though Func::reset() doesn't use the template variable T, I need
// to declare "template<typename T>" before the definition, and <T> after the class
// name Func, if we define a member function outside of the class like this case.

template<typename T>
void Func<T>::set_func(Fct f, double r1, double r2, Point orig,
		    int count, double xscale, double yscale, T shift)
// Note: default arguments are given in the declaration, not definition
// I think member initializer list can be used only in constructors
{
  // if only the function is provided (and the rest parameters are the same as
  // their default values), use previously set parameters
  if(r1==def_r1 && r2==def_r2 && orig==def_orig && count==def_count &&
     xscale==def_xscale && yscale==def_yscale && shift==def_shift){
    func = f;
    // and do nothing to the other parameters
  }
  else{
    func = f;
    xrange1 = r1;
    xrange2 = r2;
    origin = orig;
    num_points = count;
    this->xscale = xscale;
    this->yscale = yscale;
    this->shift = shift;
    // to stress that the variables are member variables of this object, not the
    // parameters of this function, I attached this->
  }
  
  reset();			// for safety, reset before adding new points
  add_points();
}

// copied from Shape::draw_lines()
  // just replaced points vector with pv
template<typename T>
void Func<T>::draw_lines() const
{
    if (fill_color().visibility()) {
        fl_color(fill_color().as_int());
        fl_begin_complex_polygon();
        for(int i=0; i<pv.size(); ++i){
            fl_vertex(pv[i].x, pv[i].y);
        }
        fl_end_complex_polygon();
        fl_color(color().as_int());    // reset color
    }

    if (color().visibility() && 1<pv.size())    // draw sole pixel?
        for (unsigned int i=1; i<pv.size(); ++i)
            fl_line(pv[i-1].x,pv[i-1].y,pv[i].x,pv[i].y);
}

// explicit instantiations of template class Func
template class Func<double>;
template class Func<float>;
template class Func<int>;
// These explicit instantiations must come AFTER the definitions of the member
// functions, otherwise the actual classes are not created.
// These explicit instantiations are one solution for defining template member functions
// separately from the declaration. Otherwise, in the linking phase, since the actual
// classes are not created without these, the call to Func<double> in main.cpp fails.
// For the details and another solution, see
// https://stackoverflow.com/questions/495021/
// and
// https://isocpp.org/wiki/faq/templates#templates-defn-vs-decl

//------------------------------------------------------------------------------

bool can_open(const string& s)
// check if a file named s exists and can be opened for reading
{
    ifstream ff(s);
    return ff.is_open();
}

//------------------------------------------------------------------------------

#define ARRAY_SIZE(a) (sizeof(a)/sizeof((a)[0]))

Suffix get_encoding(const string& s)
{
    struct SuffixMap 
    { 
        const char*      extension;
        Suffix suffix;
    };

    static SuffixMap smap[] = {
        {".jpg",  Suffix::jpg},
        {".jpeg", Suffix::jpg},
        {".gif",  Suffix::gif},
    };

    for (int i = 0, n = ARRAY_SIZE(smap); i < n; i++)
    {
        int len = strlen(smap[i].extension);

        if (s.length() >= len && s.substr(s.length()-len, len) == smap[i].extension)
            return smap[i].suffix;
    }

    return Suffix::none;
}

//------------------------------------------------------------------------------

// somewhat over-elaborate constructor
// because errors related to image files can be such a pain to debug
Image::Image(Point xy, string s, Suffix e)
    :w(0), h(0), fn(xy,"")
{
    add(xy);

    if (!can_open(s)) {    // can we open s?
        fn.set_label("cannot open \""+s+'"');
        p = new Bad_image(30,20);    // the "error image"
        return;
    }

    if (e == Suffix::none) e = get_encoding(s);

    switch(e) {        // check if it is a known encoding
    case Suffix::jpg:
        p = new Fl_JPEG_Image(s.c_str());
        break;
    case Suffix::gif:
        p = new Fl_GIF_Image(s.c_str());
        break;
    default:    // Unsupported image encoding
        fn.set_label("unsupported file type \""+s+'"');
        p = new Bad_image(30,20);    // the "error image"
    }
}

//------------------------------------------------------------------------------

// For Exercise 7 of chapter 16
void Image::resize(int w, int h){
  Fl_Image *tmp;
  tmp = p;			// to delete original Fl_Image, store it temporarily
  p = p->copy(w,h);
  delete tmp;
}


void Image::draw_lines() const
{
    if (fn.label()!="") fn.draw_lines();

    if (w&&h) // if w > 0 and h > 0
      p->draw(point(0).x,point(0).y,w,h,cx,cy); // uses Fl_Image::draw()
      // point(0) is the top left corner of the original image, cx, cy are the top
      // left corner of the cropped image
    else
        p->draw(point(0).x,point(0).y); 
}

//------------------------------------------------------------------------------

// exercise 1
void Arc::draw_lines() const
{
    if (color().visibility())
      fl_arc(point(0).x,point(0).y,w+w,h+h, sd, ed);
}

//------------------------------------------------------------------------------

// exercise 2
void Arc::update_arc(Point p, int ww, int hh, int ssd, int eed){
  add(Point{p.x-ww,p.y-hh});
  //add(Point{p.x-w,p.y-h});
  // it seems member variables w and h are not initialized to 0, instead they are
  // initialized to some random values, so if I use w and h here in add() instead
  // of ww and hh, a point with strange position is added like (-109259454, 104),
  // hence makes strange
  // output
  // hh,
  w = ww;
  h = hh;
  sd = ssd;
  ed = eed;

  //cout << point(0) << endl;
  //cout << "(" << point(0).x << ", " << point(0).y << ")" << endl;
  // Mysteriously, the overloaded operator << for ostream& and Point cannot be used
  // here, although it can be used in Box::Box
  // cout << p << endl;
  // <- it turned out that it worked for Point variable p. So it seems a Point
  //   variable and point(0) or Point{,} are treated differently
  // <- this is because Point variable is a lvalue (can be reused for storing new
  // values), whereas point(0) and Point{,} are rvalues. To reference an rvalue,
  // use const-reference, or use rvalue reference (&&)
}

Box::Box(Point p, int ww, int hh){
  // create 4 Arcs and add 4 Lines to the Lines class object
  // Let's set the width and height of the 4 Arcs to 1/10 of the total width and
  // height
  int arc_w{static_cast<int>(ww/10.0)}, arc_h{static_cast<int>(hh/10.0)};
  Point tl, tr, bl, br;			   // top right, bottom left, bottom right

  //cout << arc_w << ", " << arc_h << endl;
  
  // set the center of each Arc object
  tl.x = p.x + static_cast<int>(arc_w/2.0);
  tl.y = p.y + static_cast<int>(arc_h/2.0);
  tr.x = p.x + ww - static_cast<int>(arc_w/2.0);
  tr.y = tl.y;
  br.x = tr.x;
  br.y = p.y + hh - static_cast<int>(arc_h/2.0);;
  bl.x = tl.x, bl.y = br.y;

  /*
  cout << tl << endl;
  cout << tr << endl;
  cout << br << endl;
  cout << bl << endl;
  */
  
  a1.update_arc(tl, static_cast<int>(arc_w/2.0),
		static_cast<int>(arc_h/2.0), 90, 180); // top left corner
  // Notice: Ellipse is defined by the center and distances from the center to the
  // farthest points in x and y direction, not the width and height of the smallest
  // surrounding rectangular box. Thus we need to divide arc_w and arc_h by 2
  a2.update_arc(tr, static_cast<int>(arc_w/2.0),
		static_cast<int>(arc_h/2.0), 0, 90);
  a3.update_arc(br, static_cast<int>(arc_w/2.0),
		static_cast<int>(arc_h/2.0), 270, 360);
  a4.update_arc(bl, static_cast<int>(arc_w/2.0),
		static_cast<int>(arc_h/2.0), 180, 270);

  // add 4 sides of Line
  ls.add(Point{p.x, tl.y}, Point{p.x, bl.y});	// left side
  ls.add(Point{tl.x, p.y}, Point{tr.x, p.y});	// top side
  ls.add(Point{p.x+ww, tr.y}, Point{p.x+ww, br.y});	// right side
  ls.add(Point{tr.x, p.y+hh}, Point{bl.x, p.y+hh});	// bottom side

  // to avoid segmentation fault when the text is not specified, I add a point
  // whether or not a text is displayed
  add(Point{p.x+5, p.y+hh-7});
  set_label("");
}

// exercise 6
// in the 2nd constructor of Box, by using "delegation", we can avoid code
// duplicate of the constructors
// https://stackoverflow.com/questions/308276/
  Box::Box(Point p, int ww, int hh, const string& s)
    : Box(p, ww, hh)
      //Text(Point{p.x+15, p.y+hh-15}, s)
      // it seems when using delegation of a constructor, in the initializer,
      // we can write only that delegated constructor
  {
    // Text(Point{p.x+15, p.y+hh-15}, s);
    // Base classes' constructors are not inherited. So this statement is
    // similar to "string";, which does nothing except creating a temporary
    // object and that object is not used at all.
    // Moreover, even if base classes' constructors are inherited with
    // "using Text::Text;" statement (p456), that simply adds Text's
    // constructors in the name of Box, i.e. Text(Point x, const string& s)
    // -> Box::Box(Point x, const string& s) and Text() -> Box::Box(), so
    // in either way, constructor Text(...) cannot be used in Box class

    //add(Point{p.x+15, p.y+hh-15});
    // this is added as 2nd point in this object, so to call it later, point(1)
    // Thus, I need to overwrite draw_lines() function to refer to a correct
    // point
    // <- no, Box(p, ww, hh) never adds a point to this object, so this point
    // is added as the 1st point
    // <- if this constructor is not called and no point is added to this
    // object, in Box::draw_lines(), segmentation fault error occurs (due to
    // trying to access a point with point(0) without any point added). So
    // to avoid this case's segmentation fault, I moved this add() statement
    // inside the 1st constructor
    //lab = s;
    // <- although this is public inheritance from Text, private members of
    // Text are never accessible from the base class, except through calling
    // public member functions
    set_label(s);
  }

void Box::draw_lines() const
{
  // draw all the member objects
  a1.draw_lines(), a2.draw_lines(), a3.draw_lines(), a4.draw_lines();
  ls.draw_lines();

  // the following is a copy of Text::draw_lines(). Since in the current
  // structure of Box class, we don't have Text object inside it, rather,
  // we inherit it to Box class (so that we can use public member functions of
  // Text in Box class), we can't do text.draw_lines() as above 5 objects
  /*
  int ofnt = fl_font();
  int osz = fl_size();
  fl_font(font().as_int(),font_size());
  fl_draw(label().c_str(),point(0).x,point(0).y);
  fl_font(ofnt,osz);
  */
  Text::draw_lines();
}

//------------------------------------------------------------------------------

  // exercise 3
Arrow::Arrow(Point p1, Point p2){
  add(p1);
  add(p2);			// like Line's constructor

  Point p3, p4;		    // points of the other 2 points in the arrow head
  const int dist1{8}, dist2{5};
  // dist1 is the distance from p2 to a point on line p1p2 which is on the
  // bottom of the arrow head (say this point as pa).
  // dist2 is the distance from pa to p3, which is the right angular point
  // of the arrow head (dist2 is the same as the distance from pa to p4, which
  // is the left angular point of the arrow head).
  
  if(p1.y == p2.y){
    // if the line is horizontal, my original way cannot be used, so I need
    // to separate this case
    p3.x = p2.x-dist1, p3.y = p2.y-dist2;
    p4.x = p2.x-dist1, p4.y = p2.y+dist2;    
  }
  else if(p1.x == p2.x){
    // my original way won't work either when the line p1p2 is virtical
    // because when solving for a (slope of p1p2), the denominator becomes 0
    p3.x = p2.x-dist2, p3.y = p2.y-dist1;
    p4.x = p2.x+dist2, p4.y = p2.y-dist1;
  }
  else{
    double k;			// slope of lines perpendicular to line p1p2
    k = (p1.x-p2.x)/(static_cast<double>(p2.y)-p1.y);
    // how this expression is derived is in my note
    //cout << "### k == " << k << endl;
    
    double a, b;			// y=ax+b, the line p1p2
    a = (p2.y-p1.y)/(static_cast<double>(p2.x)-p1.x);
    b = static_cast<double>(p2.x*p1.y-p1.x*p2.y)/(p2.x-p1.x);
    // how this expression is derived is in my note
    //cout << "### a, b == " << a << ", " << b << endl;
    double ax;			// Pa's x relative to p2
    ax = sqrt(static_cast<double>(pow(dist1,2)) / (pow(a,2)+1));
    // if(ax < 1)
    //   ax = 1;
    // <- since point pa is an intermediate product, which is not displayed
    // on the screen, its x and y coordinates need not be integers. Since
    // this rounding of ax to 1 can enlarge the arrow head greatly (when ax is
    // really small), it is desirable not to round variables as much as
    // possible, in order to reduce such a round off distortion.
    //Point pa;
    double pax{0}, pay{0};	// point pa's x and y coordinates. Point pa
    // is not displayed on the screen, so they need not be integers
    if((a<0 && p2.x>p1.x) || (a>0 && p2.x>p1.x))
      pax = p2.x - ax;
    else
      pax = p2.x + ax;
    pay = a*pax + b;
    
    double b2;
    // y=kx+b2, the line perpendicular to line p1p2, and passing point pa
    b2 = pay - k*pax;
    //cout << "### b2 == " << b2 << endl;
    double x34;			// p3 or p4's x relative to pa
    // since this x34 affects p3 and p4's positionings, and p3 and p4 need
    // be different points from p2 and pa (so their x coordinates need to be
    // different from p2 and pa), it needs such a rounding
    x34 = sqrt(static_cast<double>(pow(dist2,2)) / (pow(k,2)+1));
    if(x34<1)
      x34 = 1;
    p3.x = static_cast<int>(pax+x34), p3.y = static_cast<int>(k*p3.x+b2);
    p4.x = static_cast<int>(pax-x34), p4.y = static_cast<int>(k*p4.x+b2);
  }
  
  arrow_head.add(p2);
  arrow_head.add(p3);
  arrow_head.add(p4);

  // make the color and fill color the same as the arrow's color
  arrow_head.set_fill_color(this->fill_color());
  arrow_head.set_color(this->color());

  //cout << color().visibility() << endl;
  // this statement worked even if Shape class is inherited in private mode,
  // where color() is private, and its returned variable "lcolor" is not
  // supposed to be accessible (because it is private of the base class which
  // is inherited in private mode).
  // So I think even in private mode, the base class's private member variables
  // are accessible through the base class's member functions
}

void Arrow::draw_lines() const {
  Shape::draw_lines();		// draw a line from the points in this object
  // to use Shape class's draw_lines(), which is inherited as private member
  // function to Arrow class (due to private inheritance, public members in
  // the base class become private in the derived class), and which is
  // overwritten by this Arrow::draw_lines(), I use the full name
  // <- when inheriting in struct, the default inheritance mode becomes public
  // , so Shape::draw_lines() is a public member in Arrow class

  // draw the arrow head
  arrow_head.draw_lines();	// draw the outline of the arrow head
  // fill inside the arrow head
  if(arrow_head.fill_color().visibility()){
    fl_color(arrow_head.fill_color().as_int());
    //fl_color(Color{Color::red}.as_int());
    fl_polygon(arrow_head.point(0).x, arrow_head.point(0).y,
	       arrow_head.point(1).x, arrow_head.point(1).y,
	       arrow_head.point(2).x, arrow_head.point(2).y);
    
  }
}

void Arrow::set_color(Color col){
  Shape::set_color(col);	// set color for the 2 lines
                                // (line part of the Arrow)

  // set arrow_head's color and fill color as well
  arrow_head.set_color(col);
  arrow_head.set_fill_color(col);
}


// exercise 7
Color::Color(int r, int g, int b)
  : v(visible)
{
  // https://www.fltk.org/doc-1.3/group__fl__attributes.html
  // set the RGB color to c
  fl_color(static_cast<uchar>(r), static_cast<uchar>(g), static_cast<uchar>(b));
  c = fl_color();
}


  // exercise 10
Regular_polygon::Regular_polygon(Point center, int num_sides, double dist){
  // inspired by: https://stackoverflow.com/questions/3436453/
  const double pi{3.1415};
  const double deg_per_side{360.0/num_sides}; // degrees per one side 

  Point p;
  for(int i=0; i<num_sides; i++){
    // the following way starts a point at the position of 3 o'clock
    p.x = static_cast<int>(center.x + dist*cos(pi*(i*deg_per_side/180.0)));
    p.y = static_cast<int>(center.y + dist*sin(pi*(i*deg_per_side/180.0)));

    // the following way starts a point a little above from 3 o'clock, by
    // the degree deg_per_side/2, so that when, for example, # of sides is 5,
    // the shape becomes line symmetry
    // <- it turns out it didn't become line symmetry
    /*
    p.x = static_cast<int>(center.x +
			   dist*cos(pi*((deg_per_side/2+i*deg_per_side)/180.0)));
    p.y = static_cast<int>(center.y +
			   dist*sin(pi*((deg_per_side/2+i*deg_per_side)/180.0)));
    */
    add(p);
    // this add() must be Polygon::add(), not Shape::add(), since
    // Polygon::add() is "closer" to Regular_polygon, I think (I am not sure)
  }
}

// exercise 18
Poly::Poly(initializer_list<Point> lst){
  for(auto p : lst){
    add(p);
    // since Poly's base class is Polygon, this add() is Polygon::add(), not
    // Shape::add() (Polygon::add() overrides Shape::add())
  }
}


// exercise 19
Star::Star(Point center, int num_points, double dist){
  // as in Exercise 10 (Regular_polygon), I will use cos and sin to get points
  // Define 2 circles, one is of radius dist, the other is of radius dist/2
  // The former circle is used for getting the outer points, the latter is for
  // inner points of a star
  const double pi{3.1415};
  const double deg_per_point{360.0/num_points};
  // degree between outer points (== degree between inner points)

  Point p;
  int j;
  for(int i=0; i<2*num_points; i++){
    if(i%2==0){			// add an outer point
      j = i/2;
      p.x = static_cast<int>(center.x + dist*cos(pi*(j*deg_per_point/180.0)));
      p.y = static_cast<int>(center.y + dist*sin(pi*(j*deg_per_point/180.0)));
    }
    else{			// add an inner point
      j = i/2;
      p.x = static_cast<int>(center.x +
			     (dist/2)*cos(pi*((deg_per_point/2 + j*deg_per_point)/180.0)));
      p.y = static_cast<int>(center.y +
			     (dist/2)*sin(pi*((deg_per_point/2 + j*deg_per_point)/180.0)));
    }

    add(p);			// Polygon::add(), not Shape::add()
  }
  
}


Smily::Smily(Point p, int rr)
  : Circle(p, rr)
{
  // store 3 corners, one for the left eye, one for the right eye, and
  // one for the mouth
  add(Point{static_cast<int>(0.8*rr*cos(deg2rad(210))+p.x),
	    static_cast<int>(0.8*rr*sin(deg2rad(210))+p.y)}); // left eye
  add(Point{static_cast<int>(0.8*rr*cos(deg2rad(330))+p.x-0.5*rr),
	    static_cast<int>(0.8*rr*sin(deg2rad(330))+p.y)});   // right eye
  add(Point{static_cast<int>(p.x-rr*0.35), p.y});	   // mouth
}

void Smily::draw_lines() const {
  // first, draw the normal circle
  Circle::draw_lines();;    // bound at compile time (due to direct call)

  if(color().visibility()){
    // then, draw left and right eyes
    fl_arc(point(1).x, point(1).y, 0.5*radius(), 0.5*radius(),
	   0, 180);
    // I think even if variable r is private in the base class hence
    // not accessible from this derived class, we can still access it
    // via member functions of Circle which access r
    fl_arc(point(2).x, point(2).y, 0.5*radius(), 0.5*radius(),
	   0, 180);

    // draw mouth
    fl_arc(point(3).x, point(3).y, 0.7*radius(), 0.7*radius(),
	   180, 360);   
  }
}

Frowny::Frowny(Point p, int rr)
  : Circle(p, rr)
{
  // store 3 corners, one for the left eye, one for the right eye, and
  // one for the mouth
  add(Point{static_cast<int>(0.8*rr*cos(deg2rad(210))+p.x+0.25*rr),
	    static_cast<int>(0.8*rr*sin(deg2rad(210))+p.y)}); // left eye
  add(Point{static_cast<int>(0.8*rr*cos(deg2rad(330))+p.x-0.5*rr),
	    static_cast<int>(0.8*rr*sin(deg2rad(330))+p.y)});   // right eye
  add(Point{static_cast<int>(p.x-rr*0.35), static_cast<int>(p.y+0.2*rr)});   // mouth
}

void Frowny::draw_lines() const {
  // first, draw the normal circle
  Circle::draw_lines();;    // bound at compile time (due to direct call)

  if(color().visibility()){
    // draw 2 eyes
    // set the fill color of these 2 circles to the same as line color()
    // <- since filling eyes looks scary, I decided not to fill it
    fl_color(color().as_int());
    for(int i=1; i<3; ++i)
      fl_arc(point(i).x,point(i).y,0.2*radius(),0.2*radius(),0,360);
    
    // draw mouth
    fl_arc(point(3).x, point(3).y, 0.7*radius(), 0.7*radius(),
	   0, 180);   
  }
}

Striped_rectangle::Striped_rectangle(Point xy, int ww, int hh)
  : Rectangle(xy, ww, hh)
{
  // by Rectangle(xy, ww, hh), the top left corner xy is stored in
  // point(0)
  // store stripe lines
  for(int y=point(0).y+2; y<point(0).y+hh; y+=2){
    ls.add(Point{point(0).x, y}, Point{point(0).x+ww-1, y});
  }
  
}

Striped_rectangle::Striped_rectangle(Point x, Point y)
  : Striped_rectangle(x, y.x-x.x, y.y-x.y)
{}
// using delegation of constructors (see Box::Box() in this file)

void Striped_rectangle::draw_lines() const {
  // draw outline
  if (color().visibility()) {    // lines on top of fill
    fl_color(color().as_int());
    fl_rect(point(0).x,point(0).y,width(),height());
  }

  ls.draw_lines();
  
}

// Ex 6
Striped_circle::Striped_circle(Point p, int rr)
  : Circle(p, rr)
{
  // store Lines
  // point(0) is the top left corner of the enclosing square
  Point p1, p2;
  for(int y=point(0).y; y<point(0).y+2*rr; y+=2){
    // x = Cx +/- sqrt(r^2 - (y-Cy)^2): equation of a circle
    p1.x = p.x - static_cast<int>(sqrt(rr*rr - (y - p.y)*(y - p.y)))
      -1;			// -1 is for avoiding the circle outline
    p2.x = p.x + static_cast<int>(sqrt(rr*rr - (y - p.y)*(y - p.y)))
      -1;
    p1.y = y, p2.y = y;
    ls.add(p1, p2);
  }
}

void Striped_circle::draw_lines() const
{
  // to avoid the circle being filled with a color, I don't use
  // Circle::draw_lines() directly
  if (color().visibility()) {
    fl_color(color().as_int());
    fl_arc(point(0).x,point(0).y,2*radius(),2*radius(),0,360);
  }
  ls.draw_lines();
}


// Ex 7
void Striped_closed_polyline::finish(){
  // get N line equations, x = ay + b
  //vector<tuple<double, double, bool>> vlp; // tuple of parameters
  // the first and second elements are a and b, and the last is for
  // specifying whether the line is horizontal or not (when the line is
  // horizontal, we cannot represent the line with the form x=ay+b)
  // <- Wait, we don't need to store such horizontal lines, in the first
  //    place, because our stripe is horizontal, we can always exclude
  //    horizontal lines from consideration
  //vector<pair<double, double>> vdp;
  // vdp[i].first, vdp[i].second for accessing pair elements
  vector<tuple<double, double, int, int>> vtp;
  // <- to add the x range of the line segment, I changed to tuple
  double a{}, b{}, x1{}, x2{};
  for(int i=0; i<number_of_points(); ++i){
    // number_of_points() is a member of Shape class
    if(i!=number_of_points()-1){      // except the last line
      if(point(i).y != point(i+1).y){
	// make sure the line is not horizontal (since in calculation of
	// a, 0 division happens)
	a = static_cast<double>(point(i+1).x-point(i).x) /
	  (point(i+1).y - point(i).y);
	b = point(i).x - a*point(i).y;
	x1 = min(point(i).x, point(i+1).x);
	x2 = max(point(i).x, point(i+1).x);
	vtp.push_back(make_tuple(a, b, x1, x2));
      }
      else{			// if the line is horizontal
	// y = point(i).y == point(i+1).y
	// we can skip this line, because our strip is horizontal as well
	continue;
      }
    }
    else{			// for the line of point(N-1) and point(0)
      if(point(i).y != point(i+1).y){
	a = static_cast<double>(point(i).x-point(0).x) /
	  (point(i).y - point(0).y);
	b = point(i).x - a*point(i).y;
	x1 = min(point(i).x, point(0).x);
	x2 = max(point(i).x, point(0).x);
	vtp.push_back(make_tuple(a, b, x1, x2));
      }
      else{			// if the line is horizontal
	// do nothing
      }
    }
  } // for(i...)

  // check
  /*
  for(int i=0; i<vtp.size(); ++i){
    cout << "a, b, x1, x2 = " << get<0>(vtp[i]) << ", " << get<1>(vtp[i])
	 <<  ", " << get<2>(vtp[i]) <<  ", " << get<3>(vtp[i]) << endl;
  }
  */

  // next, get the minimal enclosing box of the closed_polyline
  int xmax{}, xmin{10000}, ymax{}, ymin{10000};
  for(int i=0; i<number_of_points(); ++i){
    xmax = max(point(i).x, xmax); // using std::max()
    xmin = min(point(i).x, xmin);
    ymax = max(point(i).y, ymax);
    ymin = min(point(i).y, ymin);
  }

  // from ymin to ymax, scan y value and if the line's x value at that y
  // is within (xmin, xmax), that point is in the stripe lines
  for(int y=ymin+2; y<ymax; y+=2){
    vector<Point> vp;
    for(int i=0; i<vtp.size(); ++i){
      double x;
      x = get<0>(vtp[i]) * y + get<1>(vtp[i]);
      // if this x is on the line segment, add the point
      // Note: if x is rounded to int at this point, it falsely adds
      // an unexpected point, e.g. when the x-range is: (450,500),
      // Point (500.1764, 652) -(rounded)> (500,652), and this point
      // (500, 652) is unexpectedly added, and cause an error.
      // So use floating point precision until the comparison to the
      // x-range is done.
      if(x >= get<2>(vtp[i]) && x <= get<3>(vtp[i])){
	vp.push_back(Point{static_cast<int>(x), y});
      }
    }

    // sort by points' x value
    // https://stackoverflow.com/questions/2999135/
    sort(vp.begin(), vp.end(),
	 [](const Point& p1, const Point& p2){
	   return p1.x < p2.x;
	 });

    // delete duplicate points located at either (left or right) side
    // when vp.size() is odd (when vp.size() is even, it is likely that
    // the pairs of points point to the same vertex of a closed polygon)
    if(vp.size() % 2 != 0){
      // left edge
      if(vp[0].x == vp[1].x){
	vp.erase(vp.begin());
      }
      // right edge
      if(vp[vp.size()-1].x == vp[vp.size()-2].x){
	vp.erase(vp.end());
      }
    }

    if(vp.size() % 2 != 0){
      cerr << "y==" << y << ", vp.size() == " << vp.size() << endl;
      error("Error: unexpected number of points in Striped_closed_polyline");
    }
    
    for(int i=0; i<vp.size(); i+=2){
      ls.add(vp[i], vp[i+1]);
    }
  }

}

void Striped_closed_polyline::draw_lines() const
{
  Closed_polyline::draw_lines();
  ls.draw_lines();
}


// Ex 9
void Group::draw_lines() const
{
  /*
  cout << "\n### Group::draw_lines()\n";
  cout << "fill_color() == " << fill_color().as_int()
       << " (Color::green.as_int() == " << Color{Color::green}.as_int()
								 << ")"<< endl;
  */
  for(int i=0; i<vrs.size(); ++i){
    vrs[i].draw();
    // This call is regarded as (Shape).draw(), so this is regarded as calling
    // the function from outside of Shape?
    // Shape::draw_lines() is a protected member function, whereas Shape::draw()
    // is a public member, so it is callable
    // In draw(), it calls draw_lines(). I think this draw_lines() will refer
    // to vtable via vptr of the object vrs[i]

    // <- yes, I think so. I tested this in test_virtual_override.cpp in this
    // directory. this->draw_lines() refers to vtable via vptr, since
    // Shape::draw_lines() is declared as virtual.
    // In Group class, we can access Shape::draw_lines() via
    // this->draw_lines() since it's a protected member. But vrs[i].draw_lines()
    // is trying to call draw_lines() from outside of the object vrs[i], so
    // it generates the error.

    // For example, suppose vrs[i] is a Rectangle, and with vrs[i].draw(), when
    // calling Rectangle::draw_lines(), in there, it calls fill_color().
    // This is the same as this->fill_color(), and "this" is referring to the
    // object that calls this draw_lines() function, so this->fill_color()
    // returns vrs[i]'s fill_color, not the one from the whole Group object.
  }
}

void Group::move(int x, int y){
  for(int i=0; i<vrs.size(); ++i)
    vrs[i].move(x, y);
}

void Group::move_ith(int i, int x, int y){
  if(i<0 || i>=vrs.size())
    error("Error, in Group::move_ith(), i is not in the valid range\n");

  vrs[i].move(x, y);
}

void Group::set_color(Color cc){
  for(int i=0; i<vrs.size(); ++i)
    vrs[i].set_color(cc);
}

void Group::set_fill_color(Color fc){
  for(int i=0; i<vrs.size(); ++i)
    vrs[i].set_fill_color(fc);
}

void Group::stored_fill_color(){
  for(int i=0; i<vrs.size(); ++i)
    cout << vrs[i].fill_color().as_int() << endl;
}

void Group::set_color_with_range(int b, int e, Color cc){
  // check if the range is valid
  if(b<0 || b>vrs.size()-1 || b>e || e>vrs.size()-1)
    error("Error: in Group::set_color_with_range(), the range is invalid");

  for(int i=b; i<=e; ++i)
    vrs[i].set_color(cc);
}

void Group::set_fill_color_with_range(int b, int e, Color fc){
  // check if the range is valid
  if(b<0 || b>vrs.size()-1 || b>e || e>vrs.size()-1)
    error("Error: in Group::set_color_with_range(), the range is invalid");

  for(int i=b; i<=e; ++i)
    vrs[i].set_fill_color(fc);
}


// Ch 14 Ex 11
Binary_tree::Binary_tree(Point root_pt, int lvl)
  : node_radius{10}, level{lvl}
{
  if(lvl < 0)
    error("Error: Binary_tree level must be 0 or above");

  _store_nodes(root_pt, lvl, 't');

  // add Lines
  _store_lines(lvl, 'l');
  
}

// Ex 13
// The 3rd argument c is for switching between Lines and up or down Arrow
Binary_tree::Binary_tree(Point root_pt, int lvl, char c)
  : node_radius{10}, level{lvl}
{
  if(lvl < 0)
    error("Error: Binary_tree level must be 0 or above");

  _store_nodes(root_pt, lvl, 't');

  // add Arrows
  _store_lines(lvl, c);
  
}

// this is called from Binary_tree::_store_nodes().
// I separated this functionality from Binary_tree::_store_nodes() to switch
// different kinds of Circle (Texted_circle, Circle, Smily, and Frowny), which
// I use in Ex 16
void Binary_tree::push_back_unnamed_circle_obj(Vector_ref<Circle>& cir_vec,
					       Point center,
					       char circle_type){    
  switch(circle_type){
  case 'c':			// normal Circle
    cir_vec.push_back(new Circle{center, node_radius});
    break;
  case 't':			// Texted_circle
    cir_vec.push_back(new Texted_circle{center, node_radius});
    break;
  case 's':			// Smily
    cir_vec.push_back(new Smily{center, node_radius});
    break;
  case 'f':			// Frowny
    cir_vec.push_back(new Frowny{center, node_radius});
    break;
  default:
    error("Error in Binary_tree::push_back_unnamed_circle_obj(). circle_type argument must be either 'c/t/s/f'");
  }

}

void Binary_tree::_store_nodes(Point root_pt, int lvl, char circle_type){
  //const int node_radius{10};
  double x_offset{0.0};
  // x_offset decreases as the level increases to avoid overlap of nodes
  x_offset = node_radius*2 * pow(2, lvl-1)/4 + (pow(2, lvl-1)/4)*node_radius/2;
  const int y_offset{30};

  for(int i=0; i<lvl; ++i){
    if(i==0){			// root node
      //circle_vec.push_back(new Circle{root_pt, node_radius});
      push_back_unnamed_circle_obj(circle_vec, root_pt, circle_type);
      continue;
    }
    for(int j=0; j<pow(2,i); j++){
      int current_i {static_cast<int>(pow(2, i)) + j};
      // current node's index in circle_vec (1-indexed, not 0-indexed, since
      // 1-indexing is easier to identify its parent node (see my note))
      Point pnc{circle_vec[current_i/2 -1].center()};
      // parent node's center (current_i/2 is still 1-indexed, so by subtracting
      // 1, I return it to 0-indexing)
      if(j%2 == 0){		// left child node
	push_back_unnamed_circle_obj(circle_vec, Point{pnc.x-static_cast<int>(x_offset), pnc.y+y_offset}, circle_type);
	// to store different kinds of Circle (Texted_circle, Circle, Smily, and 
	// Frowny), which I use in Ex 16, I made this function, instead of
	// directly adding it here with circle_vec.push_back(new Texted_circle{...})
      }
      else{			// right child node
	push_back_unnamed_circle_obj(circle_vec, Point{pnc.x+static_cast<int>(x_offset), pnc.y+y_offset}, circle_type);
      }
    }
    x_offset /= 2.0;
  }
}

void Binary_tree::_store_lines(int lvl, char c){
  for(int i=1; i<lvl; ++i){
    for(int j=0; j<pow(2, i); ++j){
      int current_i {static_cast<int>(pow(2, i)) + j};
      Point pnc{circle_vec[current_i/2 -1].center()}; // parent node center
      Point cnc{circle_vec[current_i-1].center()};    // current node center
      // get the line equation connecting pnc and cnc (y=ax+b)
      double a, b;
      a = static_cast<double>(pnc.y-cnc.y) / (pnc.x-cnc.x);
      b = cnc.y - a * cnc.x;

      double x1, x2, y1, y2;	// intersections of the line and 2 circles
      if(j%2 == 0){	     // if the node is the left child of its parent node
	x1 = _get_intersec_x(a, b, circle_vec[current_i/2 -1], '-');
	y1 = a*x1 + b;
	x2 = _get_intersec_x(a, b, circle_vec[current_i -1], '+');
	y2 = a*x2 + b;
      }
      else{		// if the node is the right child of its parent node
	x1 = _get_intersec_x(a, b, circle_vec[current_i/2 -1], '+');
	y1 = a*x1 + b;
	x2 = _get_intersec_x(a, b, circle_vec[current_i -1], '-');
	y2 = a*x2 + b;
      }

      switch(c){
      case 'l':			// Lines
	// lines.add(Point{static_cast<int>(x1), static_cast<int>(y1)},
	// 	  Point{static_cast<int>(x2), static_cast<int>(y2)});
	line_vec.push_back(new Line{Point{static_cast<int>(x1),
					    static_cast<int>(y1)},
				      Point{static_cast<int>(x2),
					      static_cast<int>(y2)}});
	break;
      case 'u':
	arrow_vec.push_back(new Arrow{Point{static_cast<int>(x2),
					      static_cast<int>(y2)},
					Point{static_cast<int>(x1),
					      static_cast<int>(y1)}});
	break;
      case 'd':
	arrow_vec.push_back(new Arrow{Point{static_cast<int>(x1),
					      static_cast<int>(y1)},
					Point{static_cast<int>(x2),
						static_cast<int>(y2)}});
	break;
      default:
	error("Error in Binary_tree::_store_lines(), invalid character in the 3rd argument of the constructor (valid: \"l/u/\")");
      }
    }
  }
}

double Binary_tree::_get_intersec_x(double a, double b, const Circle &cir, char c){
  double x;
  Point center{cir.center()};
  int node_radius{cir.radius()};
  // for the following equations, see my note
  switch(c){
  case '+':			// returns the larger x value
    x = -(a*b - center.x - center.y*a) +
	  sqrt(pow(a*b-center.x-center.y*a,2) - (1+a*a)*(center.x*center.x-pow(node_radius,2)+pow(center.y-b,2))  );
    x /= 1+a*a;
    break;
  case '-':			// returns the smaller x value
    x = -(a*b - center.x - center.y*a) -
	  sqrt(pow(a*b-center.x-center.y*a,2) - (1+a*a)*(center.x*center.x-pow(node_radius,2)+pow(center.y-b,2))  );
    x /= 1+a*a;
    break;
  default:
    error("Error in Binary_tree::_get_intersec_x(). char c must be either '+' or '-'");
  }

  return x;
}

void Binary_tree::draw_lines() const {
  // draw Circles
  draw_nodes();
  // == this->draw_nodes()
  // When I don't add "virtual" keyword to the declaration of draw_nodes()
  // in Graph.h, it draws Circle nodes even if I called draw_lines() from
  // Binary_tree_tri object. This is opposite result to my understanding.
  // I thought since "this" is the pointer to Binary_tree_tri object, which
  // means the type of "this" is Binary_tree_tri*, I thought this draw_nodes()
  // would calls Binary_tree_tri::draw_nodes() even without "virtual" keyword.
  // But it called Binary_tree::draw_nodes().
  
  //lines.draw_lines();
  // changed from Lines -> Vector_ref<Line> to enable deleting all the Line
  // elements
  // if Lines are not stored in the constructor, this statement does nothing
  for(int i=0; i<line_vec.size(); ++i)
    line_vec[i].draw_lines();

  for(int i=0; i<arrow_vec.size(); ++i)
    arrow_vec[i].draw_lines();
}

void Binary_tree::set_arrow_color(Color cl){
  for(int i=0; i<arrow_vec.size(); ++i)
    arrow_vec[i].set_color(cl);
}

void Binary_tree::set_node_color(Color cl){
  for(int i=0; i<circle_vec.size(); ++i)
    circle_vec[i].set_color(cl);
}

void Binary_tree::draw_nodes() const {
  // draw Circles
  for(int i=0; i<circle_vec.size(); ++i){
    circle_vec[i].draw_lines();
  }
}


// Ex 14
void Binary_tree::add_text2node(const string& s, const string& t){
  // input example: ("llrl", "test")
  // 1st argument: identifier to spot a specific node. For how to read this label
  // , check the text of Ex 14
  // 2nd argument: the text to be added to the node

  // first check if the current tree level is enough to search with s
  if(s.size() > level || s.size() == 0)
    error("Error in Binary_tree::add_text2node(). The string length must be equal to or less than the tree level, and greater than 0");

  // second check if the search string is comprised of only 'l' and 'r'
  for(int i=0; i<s.size(); ++i){
    if(s[i] != 'l' && s[i] != 'r')
      error("Error in Binary_tree::add_text2node(). The string characters must be either 'l' or 'r'");
  }
  
  // start the search
  int ni=1;
  // pointint to the root node at first, since s of length 1 points to the root
  // (for convenience, the index is 1-indexed (later adjusted to 0-indexed))
  for(int i=1; i<s.size(); ++i){
    if(s[i] == 'l')
      ni *= 2;
    else{
      ni *= 2; ni++;
    }
  }

  ni--;				// adjust to 0-indexed
  circle_vec[ni].set_label(t);	// use Text::set_label
  // (since Texted_circle inherits both Circle and Text classes, it can use
  // Text class's member functions)
  
}

// Ch14 Ex 12
Binary_tree_tri::Binary_tree_tri(Point root_pt, int lvl)
  : Binary_tree(root_pt, lvl)
{
  // Circles for nodes are stored in circle_vec by Binary_tree(root_pt, lvl).
  // Then in this constructor, by using the stored Circles, I store new
  // Regular_polygon objects
  for(int i=0; i<circle_vec.size(); ++i){
    reg_poly_vec.push_back(new Regular_polygon{circle_vec[i].center(), 3,
					      static_cast<double>(node_radius)});
  }

  // Since the Lines are defined based on the shape of the Circles that are
  // first stored in Binary_tree(root_pt, lvl), the Lines are not suited for
  // the new Triangle nodes
}

void Binary_tree_tri::draw_nodes() const {
  for(int i=0; i<reg_poly_vec.size(); ++i)
    reg_poly_vec[i].draw_lines();
}


// Ex 14 (ch 14)
Texted_circle::Texted_circle(Point c, int rr, string s)
  : Circle(c, rr), Text(Point{c.x-5, c.y+10}, s), font_color{Color::black}
{
}

Texted_circle::Texted_circle(Point c, int rr)
  : Circle(c, rr), Text(Point{c.x-5, c.y+10}, ""), font_color{Color::black}
{
}

// I cannot use Circle::draw_lines() and (not or) Text::draw_lines(), because 
// they both use point(0) in there
void Texted_circle::draw_lines() const{
  // for Circle (Circle's center is stored at first, so it is called as point(0),
  // which means the original Circle::draw_lines() can be used)
  Circle::draw_lines();

  // for Text part, since Text::draw_lines() also use point(0), but Text's Point
  // is stored in point(1) in this class, I cannot directly use
  // Text::draw_lines().
  // So I copied the content of Text::draw_lines(), changing the point
  // index, fnt -> font(), fnt_sz -> font_size(), and lab -> label()
  // (since fnt, fnt_sz, and lab are
  // private members of one of the base classes Text, we cannot access them
  // directly from the derived class, except through member functions of Text
  // which access them)
  if(this->font_size()){
    // add a kind of visibility in the Text part as well, for Exercise 14 ch19. When the font
    // size is 0, the text becomes invisible, in calling redraw(); in a Window class
    int ofnt = fl_font();
    int osz = fl_size();
    fl_color(font_color.as_int());	// added for Ex 14 of ch19
    fl_font(font().as_int(),font_size());
    fl_draw(label().c_str(),point(1).x,point(1).y);
    fl_font(ofnt,osz);
  }
}


// Ex 16
// To store circle type and line type in the private members in member
// initializers, I define these constructors again
Switch_nodes_Binary_tree::Switch_nodes_Binary_tree(Point root_pt, int lvl)
  : Binary_tree(root_pt, lvl), circle_type{'t'}, line_type{'l'}
{
}

Switch_nodes_Binary_tree::Switch_nodes_Binary_tree(Point root_pt, int lvl,
						   char al)
  : Binary_tree(root_pt, lvl, al), circle_type{'t'}, line_type{al}
{
}

Switch_nodes_Binary_tree::Switch_nodes_Binary_tree(Point root_pt, int lvl,
						   char al, char cir_type)
{
  // It turned out that I cannot initialize base class members in the member
  // initializers of derived classes, so I initialize them here
  // https://stackoverflow.com/questions/18479295/
  node_radius = 10;
  level = lvl;
  
  if(lvl < 0)
    error("Error: Binary_tree level must be 0 or above");

  _store_nodes(root_pt, lvl, cir_type);

  // I place this because the given circle_type might be invalid.
  // Since _store_nodes() contains error checking of whether the given
  // circle_type is valid or not, so if it passes the function safely, it means
  // this circle_type is valid.
  circle_type = cir_type;
  
  // add Arrows
  _store_lines(lvl, al);
  // al: 'u/d/l' for specifying Lines or up/down Arrows
  
}

void Switch_nodes_Binary_tree::switch_node_type(char cir_type){
  Point root_pt{circle_vec[0].center()};
  
  circle_vec.clear();

  _store_nodes(root_pt, level, cir_type);

  circle_type = cir_type;
}

void Switch_nodes_Binary_tree::recreate_bin_tree(int lvl){
  if(lvl < 0)
    error("Error in Switch_nodes_Binary_tree::recreate_bin_tree(). The argument must be 0 or greater int");
  // Since this lvl error checking is done in Binary_tree's constructors, not
  // in _store_nodes() nor _store_lines(), I need to do it here as well
  
  Point root_pt{circle_vec[0].center()};
  
  circle_vec.clear();
  line_vec.clear();
  arrow_vec.clear();

  _store_nodes(root_pt, lvl, circle_type);

  _store_lines(lvl, line_type);
}

//------------------------------------------------------------------------------

// Ex 10 of Chapter 15
Scatter_graph::Scatter_graph(Point origin, const vector<pair<double, double>> pv, 
			     double xscale, double yscale,
			     const vector<double> &vv, const vector<string> &sv){

  // first decide radiuses of each scatter point
  vector<double> svradius_vec;	// scatter points' radiuses
  constexpr int min_rad{3};	// minimum radius
  // the radius of a scatter point is decided by how many times the value assigned to
  // that point is larger than the minimum value
  const double min_val{*min_element(vv.begin(), vv.end())};
  // min_element() is from std library, and returns an iterator, which I guess is like
  // a pointer. So *min_element() gets the actual minimum value
  for(int i=0; i<pv.size(); ++i){
    if(i>=vv.size()){
      // if later sc points are not provided with their values, set their radiuses to
      // the minimum radius
      svradius_vec.push_back(min_rad);
    }
    else{
      svradius_vec.push_back(min_rad*(vv[i]/min_val));
    }
  }
  
  for(int i=0; i<pv.size(); ++i){
    scpoints.push_back(new Texted_circle{Point{origin.x+int(pv[i].first*xscale),
						 origin.y-int(pv[i].second*yscale)},
					   (int)(svradius_vec[i])});
    // -int(pv[i].second*yscale) since in Window, negative y points upwards
  }

  // then set labels
  for(int i=0; i<sv.size(); ++i){
    scpoints[i].set_label(sv[i]);
  }
}

void Scatter_graph::draw_lines() const {
  for(int i=0; i<scpoints.size(); ++i)
    scpoints[i].draw_lines();
}

} // of namespace Graph_lib



