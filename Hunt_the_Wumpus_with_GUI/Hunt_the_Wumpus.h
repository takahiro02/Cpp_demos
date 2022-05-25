
#include "./std_lib_facilities.h"
#include<limits>		// for std::numeric_limits::lowest()
#include<algorithm>		// for std::random_shuffle()
#include <chrono>       // std::chrono::system_clock for random engine seed in Ex 12

#include "GUI.h"		// for GUI of the game

namespace Hunt_the_Wumpus{
// Command class is used for reading commands by a player of Hunt the Wumpus game, such as
// "s13-4-3".
  // I first place Command class inside HW_game class's private section, but if I do so, in the
// operator overload of >>, I cannot specify Command class as its 2nd argument. Probably I can
// still place Command class inside HW_game class's public section, and call it in the 2nd
// argument of >>, but I'd rather choose to place it outside of HW_game class.
struct Command{		// e.g. s13-4-3
    char move;			// either 's' (shoot) or 'm' (move)
    int room_nums[3];		// At most 3 room numbers are specified
    // In 'm', room_nums[0] is the next room to move to
    // In 's', room_nums[0,1,2] specifies the rooms the arrow moves along
    // If illegal room numbers are specified in 's', they are replaced with random valid
    // (adjacent) room numbers.
};

 istream& operator>>(istream& is, Command &cmd);

 ostream& operator<<(ostream& os, Command &cmd);
 // ===========================================================================================

 struct Cave {
  Cave(ostream& ost=cout);
  void print(ostream& ost);
  // print the state of the Cave, mainly for debug
   void print(){print(os);}
   // To use Cave's member variable "os" (ostream&) as print()'s default argument, I made 1
   // overloading of print without arguments. This is done to use a different ostream for
   // debug (print()) output for HW_game_window. Using a member variable for a member function's
   // default argument caused an error (print(ostream& ost = os) caused an error).
   // This overloading method came from https://stackoverflow.com/questions/9286533/

  int put_player_initial();
  // put the player to the Cave for its initial position. Avoid rooms with Wumpus, Pits, and
  // Bats. The pits and bats can be avoided by using index_perm in this class

  void adjacent_rooms(int rn, int (&adj_rms)[3]){
    // This is how we use a reference to an array. But using vector is better.
    if(rn<1 || rn>20)
      error("Error in Cave::adjacent_rooms(int). The argument int must be in [1,20]");
    adj_rms[0] = rooms[rn].t1, adj_rms[1] = rooms[rn].t2, adj_rms[2] = rooms[rn].t3;
    // I first used tuple, but since tuple cannot be accessed by using index variable i
    // (e.g. int i=0; get<i>(adj_rms) generates an error), I switched to using a reference
  }
  // print signs of dangers in adjacent rooms
  void notify_dangers(int rn);

  // Moves the player according to the destination
   bool move_player(int move_dest, int &ply_rn);  

  int wumpus_startled(bool is_player=true);
  // It is called when a player enters the room with the Wumpus. The Wumpus is startled by the
  // entrance, and with 50% of chance, it either stays in the same room or moves to an adjacent
  // room.
  // Later I made this member public, because I want to call this from HW_game class (in
  // HW_game::play(), where an arrow shooting is processed)

  bool shoot_a_room(int shot_rn);

  int clear();
  // Reset the condition of the cave, i.e. shuffle index_perm again, and assign pits, bats,
  // and the Wumpus again.

   // functions to let users of Cave class know where these dangers are located.
   // They are for Ex 14, ch19, used in HW_game_window::ready_pressed().
   void get_bat_rooms(int (&bat_rms)[2]) const {
     bat_rms[0] = bat_rn[0];
     bat_rms[1] = bat_rn[1];
   }
   void get_pit_rooms(int (&pit_rms)[2]) const {
     pit_rms[0] = pit_rn[0];
     pit_rms[1] = pit_rn[1];
   }
   void get_wumpus_room(int &wumpus_rm) const {
     wumpus_rm = wumpus_rn;
   }
private:
  struct Room {
    int t1,t2,t3;
    // indices to the rooms to which this Room is connected via the 3 tunnels
    bool bat, player, wumpus, pit;
    // indicators of these are in this Room
    Room()
      : t1{}, t2{}, t3{}
    {}
  };
  Room rooms[21];
  // the # of Rooms in a cave is pre-set to 20, from the original Hunt the Wumpus
  // To make the indices 1-indexed, I prepare 21 Rooms
  // I make Room definition and rooms private, to avoid accidental change in cave states,
  // especially the room connections, because they are supposed to be fixed

  int index_perm[20];		// a random permutation of 20 (from 1 to 20, not from 0) indices
  // used for setting room #s to bat, pit, wumpus, and the player, avoiding picking up the same
  // room # as those rooms with bat, pit (the Wumpus can be in the same room as bat or pit)
  // for the initial setting

  int bat_rn[2];		// 2 giant bats' room #s, fixed
  int pit_rn[2];		// 2 bottomless pits' room #s, fixed
  int wumpus_rn;		// the Wumpus' room #, can change
  int player_rn;		// the player's room #, can change
  // Cave class doesn't have to store the position of the player. There is no member function
  // to use that information for its process. In put_player_initial() and move_player(int),
  // they store player_rn, but in those members, player_rn is used only for lvalue, not
  // rvalue, which means those members doesn't use the information of player_rn for its
  // processes. So we can cut this variable, and modify those 2 functions not to store
  // player_rn. But for now, this is just a small piece, so I leave it.

  int bat_replacement();
  // It is called when a player enters the room with a bat. It returns a random room number
  // except the room numbers where bats exist.

   ostream& os;			/* added for Ex 14 of ch19 */
   /* Need to be reference, not ostream, because iostream's copy constructors are deleted */
   /* https://stackoverflow.com/questions/69202632/ */
};

 // ===========================================================================================
 
 struct HW_game {
  void play(bool is_debug=false);
  // play one run of Hunt the Wumpus game (until a player wins or loses), and repeat it
  // as long as the player wishes.
  // If is_debug is true, after every move of the player, print the cave's condition.

  void run();
  // repeat play()
  
 HW_game(istream& ist=cin, ostream& ost=cout)
   : cave{ost}, player_rn{cave.put_player_initial()}, is{ist}, os{ost}, WUMPUS_HUNTED{false},
     num_arrows{5}, STILL_IN_GAME{true}, PLAYER_KILLED{false}
  {
    // By the empty constructor of Cave class, at this point, in variable cave, the Wumpus,
    // 2 bats, and 2 pits are already set.
    // In addition, the player is also set to its initial position by cave.put_player_initial().
    // The function deals with overlap between the player and Wumpus.
  }

   int clear(){return cave.clear();}
   /* returns the player's next initial position */
protected:			/* I made it protected from private, to use the private member
				   functions in HW_game_window by inheriting this class */
  bool is_valid_move(Command cmd);
  // check if the destination of the move 'm' is correct

  bool shoot_rooms(Command cmd);

   /* These 2 members are added to use this class for GUI window */
   void move_1turn(Command cmd);
   bool get_command(Command& cmd); /* store the command and return true if it is a valid 
				     command, and return false otherwise */
   
  Cave cave;
  int player_rn;

   bool WUMPUS_HUNTED;
   // becomes true when the Wumpus is hunted

   int num_arrows;
   // The number of arrow the player can shoot is not mentioned either on the book or on
   // Wikipedia. But to make the game a little realistic and fun, I set a maximum number
   // of arrows. If the player runs out of all arrows, he loses.

   bool STILL_IN_GAME;		/* used in the main while loop in HW_game::play() to indicate
				   the play of the game has ended. This is set in move_1turn()*/

   bool PLAYER_KILLED;		// introduced in Ex 14 of ch19, to let HW_game_window class know
   // the last Room where the player visited and was killed. If I use player_rn being -1 for
   // checking if the player was killed or not, we cannot know the last Room he was in, if
   // a giant bat carried him to one of deadly Rooms and he died from it.
   
   /* added for Ex 14 of ch19 */
   istream& is;
   ostream& os;
   /* Need to be reference, not ostream, because iostream's copy constructors are deleted */
   /* https://stackoverflow.com/questions/69202632/ */
};

 // ===========================================================================================

 /* For GUI of this game */
 using namespace Graph_lib;

 /* The map (dodecahedron) of the game */
 struct Cave_map : Shape {
   Cave_map(Point center_point, int radius);
   void draw_lines() const;
   void clear();
   void visit(int player_rn, bool MAKE_PREV_INVISIBLE=false, bool MAKE_ADJ_INVISIBLE=false);

   void set_label(int room_num, string label){
     if(room_num < 1 || room_num > 20)
       throw runtime_error("In Cave_map::set_label(), invalid room number is input");
     room_vec[room_num].tc.set_label(label);
   }
 private:
   struct Passage{
     Line *l_ptr;     // these pointers are used to make the pointed and Lines visible
     int leading_rn;		// room number this passage connects to
     // This leading_rn is needed in Cave_map::visit(), where I need to make passages
     // that lead to an already visited room stay visible
     Passage(Line *l, int rn)
       : l_ptr{l}, leading_rn{rn}
     {}
     Passage()
       : l_ptr{nullptr}, leading_rn{-1}
     {}
   };
   struct Room {
     Texted_circle tc;

     // Line *l[3];
     Passage passages[3];
     
     Room(Point center, int radius)
       // : tc{center, radius}, l{nullptr, nullptr, nullptr}, VISITED{false}
	 // l[0]{nullptr}, l[1]{nullptr}, l[2]{nullptr} doesn't work
       : tc{center, radius}, VISITED{false}
	 // passages[3] are initialized with default (empty) constructor of Passage class
     {}
     bool VISITED;		// true if this room object is already visited, false otherwise
   };

   // get the intersection between the line connecting 2 Texted_circles' centers and 1 of the
   // 2 Texted_circles. There are 2 intersections in the line and such a circle. We choose one
   // of them by specifying which of these we choose based on the x coordinate of the 2 points
   Point get_intersec(Point center_point, int radius, double a, double b, char which);

   // get the line of given 2 Points, in the form of y = a*x + b
   tuple<double, double> get_line_formula(Point p1, Point p2){
     double a, b;
     a = static_cast<double>(p2.y - p1.y) / (p2.x - p1.x);
     b = p1.y - a * p1.x;
     return make_tuple(a, b);
   }
   
   // To store unnamed objects (e.g. room_vec.push_back(new Room{...})), I use Vector instead of
   // normal vector<>
   Vector_ref<Room> room_vec;
   Vector_ref<Line> line_vec;

   int font_size;

   int player_rn;		// current player's position
   int previous_player_rn;	// previous player's position
   // these player positions are used in visit() to fill previously visited Room in Color::white
 };

 /* This class references GUI.h's Calculator_window */
 struct HW_game_window : Graph_lib::Window, HW_game {
   /* Graph_lib:: is needed to distinguish it from typedef class FLWindow *Window */
   /* I inherited HW_game to use HW_game's protected members in this function */
   
    HW_game_window(Point tl, int w=800, int h=600, const string& title="Hunt the Wumpus");
    // tl: top left corner of the window
    
  private:
    // Widgets
    Button quit;
    In_box command_box;
    Out_box result_box;
    Button ready;		// pressing this forwards the game
   Button debug;		// pressing this flips the boolean value of is_debug
   Button reset;		// restart the game with a new initial conditions
   Out_box debug_box;		// displays debug information

   // Ex 15 ch19
   In_box room_num_box;
   In_box mark_box;
   Button put;
   void put_pressed();
   
    // since there is gui_main() in main.cpp, I don't need a wait function (in gui_main(),
    // it calls Fl::run(), and Fl::run() calls Fl::wait(), which is called in
    // Simple_window::wait_for_button() too)
    void quit_pressed(){hide();}
    void ready_pressed();	/* defined in Hunt_the_Wumpus.cpp */
   void debug_pressed(){is_debug = !is_debug;}
   void reset_pressed();
   
    istringstream iss;
    ostringstream oss;
   ostringstream oss_debug;	// ostringstream for debug window
    Command cmd;
    bool is_debug;		/* becomes true when debug mode button is pushed, and false
				   when the button is hit twice */

   Cave_map cave_map;

   // these are used to mark the rooms with a danger as VISITED as well as other rooms.
   // See my comment in ready_pressed(), in the else clause of moving 1 turn.
   int bat_rms[2];
   int pit_rms[2];
   int wumpus_rm;
  };
 
};
