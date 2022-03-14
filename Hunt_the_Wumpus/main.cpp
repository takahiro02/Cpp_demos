
#include "./std_lib_facilities.h"
#include<limits>		// for std::numeric_limits::lowest()
#include<algorithm>		// for std::random_shuffle()
#include <chrono>       // std::chrono::system_clock for random engine seed in Ex 12


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
// By the way, if I put this operator overload inside Command class, since the member functions'
// 1st argument always become "this" keyword (the Command class object that calls the member
// functions), if I want to set Command class to their 2nd argument, I cannot define it as a
// member function
istream& operator>>(istream& is, Command &cmd){
  // to deal with different lengths of inputs (e.g. s13-4-3, m12), I first get the whole line,
  // because if I don't do so, this operator waits for the next character indefinitely when
  // the player will not write any more command (e.g. s13-4 is a possible command, but since
  // one more room number is also possible, command line stops at s13-4)
  string line;
  getline(is, line);

  // Before putting a new command to cmd, delete the old command
  cmd.move = 'e';
  cmd.room_nums[0] = -1, cmd.room_nums[1] = -1, cmd.room_nums[2] = -1;
  
  // Then, use istringstream
  istringstream istr{line};
  char a;
  int rms[3]={-1, -1, -1};	// -1 means no room is specified
  istr >> a;
  if(a != 'm' && a != 's'){
    is.clear(ios_base::failbit); // set the state to fail()
    return is;
  }

  // if the action is shoot, we take at most 3 destinations
  if(a == 's'){
    // "12-11-13" or "12" or "12-11"
    int r;
    char sep;			// separator '-'
    for(int i=0; i<3; ++i){
      if(istr >> r){
	rms[i] = r;
      }
      else{
	is.clear(ios_base::failbit);
	return is;
      }

      // check if there is no character after the 3rd int
      if(i == 2 && !istr.eof()){
	char buff;	     // buffer char to flush any remaining white spaces in the last
	istr >> buff;
	// Even after reading 1 character, if istr's state is not in end-of-file, that means
	// there are some character after the 3rd int other than a whitespace
	if(!istr.eof()){
	  cout << "Maximum arror range is 3\n";
	  is.clear(ios_base::failbit);
	  return is;
	}
      }

      if(istr.eof())
	break;

      istr >> sep;
      if(sep != '-'){
	cout << "Arrow direction separator must be '-'\n";
	is.clear(ios_base::failbit);
	return is;
      }
    }

    cmd.room_nums[0] = rms[0];
    cmd.room_nums[1] = rms[1];	// might be -1
    cmd.room_nums[2] = rms[2];
  }
  else{				// if the action is move, take only 1 int
    int r;
    char buff;			// buffer char to flush any remaining white spaces in the last
    if(istr >> r){
      rms[0] = r;
    }
    else{
      is.clear(ios_base::failbit);
      return is;
    }

    // In case there is some whitespaces in the end, we read 1 char
    // (if I don't do this, in such a case, istr does not become end-of-file state)
    istr >> buff;		// this buff is used nothing
    // Even if buff reads 1 char, and that 1 char is the last element in istr, istr doesn't
    // become end-of-file state. But if buff is in string class, istr does. This is 1 slight
    // difference between reading in string or char variable.
    // E.g. "m12 1", then, if buff is in char, it reads '1', but istr's state has not yet been
    //      in end-of-file state. But if buff is in string, it reads "1", along with setting
    //      istr to end-of-file state.
    
    if(!istr.eof()){
      cout << "When the action is \"Move\", you can specify only 1 room number.\n";
      is.clear(ios_base::failbit);
      return is;
    }

    cmd.room_nums[0] = rms[0];
    cmd.room_nums[1] = -1;
    cmd.room_nums[2] = -1;
  }

  cmd.move = a;
  return is;			// reaching here means the reading succeeded
}

ostream& operator<<(ostream& os, Command &cmd){
  cout << "Command char: " << cmd.move << ", " << "cmd.room_nums[0,1,2] = " << cmd.room_nums[0]
       << ", " << cmd.room_nums[1] << ", " << cmd.room_nums[2] << endl;
  return os;
}
// ===========================================================================================

struct Cave {
  Cave();
  void print();
  // print the state of the Cave, mainly for debug

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
  int move_player(int move_dest);  

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
};
Cave::Cave(){
  // Which Room is connected to which is already decided. The structure is from the original
  // Hunt the Wumpus. Since there seems no regular pattern in the connection, I set them
  // manually.
  // Note that the indexing is 1-indexed.
  rooms[1].t1 = 2, rooms[1].t2 = 5, rooms[1].t3 = 8;
  rooms[2].t1 = 1, rooms[2].t2 = 3, rooms[2].t3 = 10;
  rooms[3].t1 = 2, rooms[3].t2 = 4, rooms[3].t3 = 12;
  rooms[4].t1 =3, rooms[4].t2 = 5, rooms[4].t3 =14;
  rooms[5].t1 = 1, rooms[5].t2 = 4, rooms[5].t3 = 6;
  rooms[6].t1 = 5, rooms[6].t2 = 7, rooms[6].t3 = 15;
  rooms[7].t1 = 6, rooms[7].t2 = 8, rooms[7].t3 = 17;
  rooms[8].t1 = 1, rooms[8].t2 = 7, rooms[8].t3 = 9;
  rooms[9].t1 = 8, rooms[9].t2 = 10, rooms[9].t3 = 18;
  rooms[10].t1 = 2, rooms[10].t2 = 9, rooms[10].t3 = 11;
  rooms[11].t1 = 10, rooms[11].t2 = 12, rooms[11].t3 = 19;
  rooms[12].t1 = 3, rooms[12].t2 = 11, rooms[12].t3 = 13;
  rooms[13].t1 = 12, rooms[13].t2 = 14, rooms[13].t3 = 20;
  rooms[14].t1 = 4, rooms[14].t2 = 13, rooms[14].t3 = 15;
  rooms[15].t1 = 6, rooms[15].t2 = 14, rooms[15].t3 = 16;
  rooms[16].t1 = 15, rooms[16].t2 = 17, rooms[16].t3 = 20;
  rooms[17].t1 = 7, rooms[17].t2 = 16, rooms[17].t3 = 18;
  rooms[18].t1 = 9, rooms[18].t2 = 17, rooms[18].t3 = 19;
  rooms[19].t1 = 11, rooms[19].t2 = 18, rooms[19].t3 = 20;
  rooms[20].t1 = 13, rooms[20].t2 = 16, rooms[20].t3 = 19;

  // For randomly picking up random indices without repetition, set a random permutation of
  // room indices (from 1 to 20)
  for(int i=1; i<21; ++i)
    index_perm[i-1] = i;
  // Be careful on the indexing of index_perm. index_perm[20] has only 20 elements, but the
  // i starts with 1. So be sure to adjust the indexing by subtracting 1. Otherwise, it accesses
  // index_perm[20], which is unknown value, and leaves index_perm[0] uninitialized. And if
  // we proceed with that unknown index_perm[0] value (e.g. 69857296), it can cause illegal
  // access error (segmentation fault) in the later processes
  
  unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
  shuffle(index_perm, index_perm+20, default_random_engine(seed));
  // shuffle the array uniformly randomly
  // Although the final element's address is index_perm+19, here index_perm+20 is used, because 
  // std::shuffle()'s range is [first, last) == [first, last-1].
  // Note: random_shuffle() is deprecated (= in later versions of C++, random_shuffle() it
  //       to be removed), so we shouldn't use it. Instead, use shuffle().
  // This piece of code is from https://www.cplusplus.com/reference/algorithm/shuffle/
  
  // Then, randomly set 2 giant bats, 2 bottomless pits, and 1 Wumpus
  // (player is not set in this class, he is set at HW_game class)
  // for 2 bats
  bat_rn[0] = index_perm[0];	// index_perm[0] is for the 1st super bat
  bat_rn[1] = index_perm[1];
  // Since index_perm is a permutation, we need not worry about the overlap of the room #s

  // for 2 pits
  pit_rn[0] = index_perm[2];
  pit_rn[1] = index_perm[3];

  // Wumpus, on the other hand, doesn't care about bats or pits, so it can overlap
  wumpus_rn = rand_int(1,20);
  // Don't attach "int" in front, of course. If doing so, a new local variable with the same
  // name as the one in this class's private section is made, and that local change doesn't
  // stick to the class member.
}

// reset the cave condition
int Cave::clear(){
  // the code in this function is directly from the constructor above
  
  unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
  shuffle(index_perm, index_perm+20, default_random_engine(seed));

  // for 2 bats
  bat_rn[0] = index_perm[0];	// index_perm[0] is for the 1st super bat
  bat_rn[1] = index_perm[1];

  // for 2 pits
  pit_rn[0] = index_perm[2];
  pit_rn[1] = index_perm[3];

  // Wumpus, on the other hand, doesn't care about bats or pits, so it can overlap
  wumpus_rn = rand_int(1,20);

  player_rn = put_player_initial();
  return player_rn;
}

void Cave::notify_dangers(int rn){
  if(rn<1 || rn>20)
    error("Error in Cave::notify_dangers(int). The argument int must be in [1,20]");

  int adj_rms[3];
  adjacent_rooms(rn, adj_rms);
  int adj_rn;
  for(int i=0; i<3; ++i){
    adj_rn = adj_rms[i];

    if(adj_rn == bat_rn[0] || adj_rn == bat_rn[1])
      cout << "\t I hear a bat\n";

    if(adj_rn == pit_rn[0] || adj_rn == pit_rn[1])
      cout << "\t I feel a breeze\n";

    if(adj_rn == wumpus_rn)
      cout << "\t I smell the wumpus\n";
  }
}

// print the state of the Cave
void Cave::print(){
  // I gave up printing the structure of the cave, since it's too much work, and little
  // return
  // Instead, I will just print where the bats, pits, and Wumpus are
  // The player's position is always printed at each turn.
  cout << "The bottomless pits are at Room " << pit_rn[0] << " and " << pit_rn[1] << endl;
  cout << "The giant bats are at Room " << bat_rn[0] << " and " << bat_rn[1] << endl;
  cout << "The Wumpus is at Room " << wumpus_rn << endl;
}

int Cave::put_player_initial(){
  player_rn = index_perm[4];
  // check if this is not overlapped with the Wumpus (it is guaranteed that this is not
  // overlapped with pits or bats by using index_perm[4])
  int i=4;
  if(player_rn == wumpus_rn)
    player_rn = index_perm[5];

  rooms[player_rn].player = true;
  return player_rn;
}

int Cave::move_player(int move_dest){
  if(move_dest < 1 || move_dest > 20)
    error("Error in Cave::move_player(), invalid destination is used");

  // First, check if there is a pit in the destination, because if a pit exists, that results
  // in the player's death instantly
  // (Even if the Wumpus is in the room, it may move to an adjacent room witn 50 % chance, so
  //  So the Wumpus existing in the destination room doesn't necessarily mean the player's
  //  death)
  if(move_dest == pit_rn[0] || move_dest == pit_rn[1]){
    cout << "\tThe player fell into a pit!\n";
    player_rn = -1;		// room -1 means the player is dead
    return -1;
  }

  // Second, check the Wumpus, bacause even if a bat and the Wumpus co-exist in the destination
  // room, if the Wumpus happens to stay in the room, the player is killed, which supercedes
  // a bat's replacement of the player
  if(move_dest == wumpus_rn){
    wumpus_rn = wumpus_startled();
    // if, even after the Wumpus is startled by the player, it stays in the same room, the
    // player is killed by it
    if(wumpus_rn == move_dest){
      cout << "\tThe Wumpus is in the room!\n";
      player_rn = -1;
      return -1;
    }
    else{
      // Do nothing.
      // Since we still need to check if a bat exists in the same room, we cannot return
      // the function here
    }
  }

  // Lastly, check if a bat is in the destination. If so, it would carry the player to a random
  // room except the current room or the room with another bat (bats keep staying in the same
  // rooms)
  if(move_dest == bat_rn[0] || move_dest == bat_rn[1]){
    int new_dest{bat_replacement()};
    cout << "\tA giant bat has carried you to Room " << new_dest << "!\n";
    player_rn = move_player(new_dest);
    // If in the new destination, a pit and/or the Wumpus exist, we have to repeat the same
    // checking process for the new destination. So move_player() is recursively called here.
    return player_rn;
  }

  // if no threat exists in the destination room, set player_rn to the destination finally
  player_rn = move_dest;
  return player_rn;
}

int Cave::wumpus_startled(bool is_player){
  // There are 2 types of the Wumpus being startled, 1st is by the player entering the Room
  // the Wumpus is in, and 2nd is by the player shooting an arrow moving across Rooms.
  // In the former case, the probability of the Wumpus moving to an adjacent Room is 50 %,
  // but in the latter case, it moves to an adjacent Room for sure (100 %).
  // The default value of "is_player" is true.

  if(wumpus_rn < 0){
    return wumpus_rn;		// if the Wumpus has been already hunted, just return the
    // same Room #, which is -1.
  }
  
  if(is_player){
    // 1st create a uniformly random number in [0,1.0)
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    default_random_engine gen(seed);
    uniform_real_distribution<double> distribution(0.0,1.0);
    double rn{distribution(gen)};
    
    if(rn < 0.5){
      int adj_rms[3];
      adjacent_rooms(wumpus_rn, adj_rms);
      double rn2{distribution(gen)}; // generate a 2nd uniformly random number in [0, 1.0)
      if(rn2<(1.0/3)) wumpus_rn = adj_rms[0];
      else if(rn2<(2.0/3)) wumpus_rn = adj_rms[1];
      else wumpus_rn = adj_rms[2];
    }
    else{				// stay in the same room 
      // do nothing
    }
  }
  else{				// the case where the arrow startled the Wumpus
    int adj_rms[3];
    adjacent_rooms(wumpus_rn, adj_rms);
    wumpus_rn = adj_rms[rand_int(0,2)];
  }

  return wumpus_rn;
}

int Cave::bat_replacement(){
  return index_perm[rand_int(2,20)];
  // index_perm[0,1] are the Room #s for the 2 giant bats. A bat moves the player randomly
  // to a Room without a bat, so choose the index from 2 to 20, inclusive.
  // (rand_int(min, max) generates a random number in [min, max], including min and max)

  // We can include index_perm[0,1] for the bats to choose a room with a bat as well.
  // If I change the code that way, Cave::move_player() works correctly, because in the
  // if-sentence for bat checking, it calls itself recursively.
}

bool Cave::shoot_a_room(int shot_rn){
  if(shot_rn < 1 || shot_rn > 20)
    error("Error in Cave::shoot_a_room(int). The room # must be in [1,20]");

  if(wumpus_rn == shot_rn){
    wumpus_rn = -1;		// Room -1 means the Wumpus has been hunted.
    return true;
  }
  return false;
}

// =========================================================================================

struct HW_game {
  void play(bool is_debug=false);
  // play one run of Hunt the Wumpus game (until a player wins or loses), and repeat it
  // as long as the player wishes.
  // If is_debug is true, after every move of the player, print the cave's condition.

  void run();
  // repeat play()
  
  HW_game()
    : cave{}, player_rn{cave.put_player_initial()}
  {
    // By the empty constructor of Cave class, at this point, in variable cave, the Wumpus,
    // 2 bats, and 2 pits are already set.
    // In addition, the player is also set to its initial position by cave.put_player_initial().
    // The function deals with overlap between the player and Wumpus.
  }
private:
  bool is_valid_move(Command cmd);
  // check if the destination of the move 'm' is correct

  bool shoot_rooms(Command cmd);
  
  Cave cave;
  int player_rn;
};

void HW_game::run(){
  char r;
  char m;
  string buff;		// buffer
  while(true){
    cout << "Do you play Hunt the Wumpus? (y/n) ";
    cin >> r;
    if(r == 'y'){
      cout << "In debug mode, or normal mode? (d/n) ";
      cin >> m;
      if(m == 'd'){
	getline(cin, buff);
	// Without getline(), the first read of Command in play() failed. I don't exactly know
	// why
	this->play(true);	// "this->" is not necessary, but for clarification
      }
      else if(m == 'n'){
	getline(cin, buff);
	this->play(false);
      }
      else
	cout << "Unknown mode is entered. Mode is 'd' or 'n'.\n";
    }
    else if(r == 'n')
      break;
    else
      cout << "Sorry, can you answer either 'y' or 'n'?\n";
  }
}

bool HW_game::is_valid_move(Command cmd){
  // If the move is shoot, always return true regardless of the destination of the shoot
  if(cmd.move == 's')
    return true;
  
  int adj_rms[3];
  cave.adjacent_rooms(player_rn, adj_rms);
  for(int i=0; i<3; ++i){
    if(adj_rms[i] == cmd.room_nums[0])
      return true;
  }
  return false;
}

bool HW_game::shoot_rooms(Command cmd){
  int current_rn{player_rn};
  int adj_rms[3];
  bool HUNTED_WUMPUS{false};
  int shot_rms[3] = {-1,-1,-1};		// for printing which rooms are actually shot
  
  for(int i=0; i<3; ++i){
    cave.adjacent_rooms(current_rn, adj_rms);

    // It's possible that the player enters only 1 or 2 room(s), e.g. s13. In that case,
    // cmd.room_nums[1,2] == -1. So break the loop once -1 is detected.
    if(cmd.room_nums[i] == -1)
      break;
    
    // check if the destination of the arrow is included in the adjacent rooms
    if(cmd.room_nums[i] == adj_rms[0] || cmd.room_nums[i] == adj_rms[1] ||
       cmd.room_nums[i] == adj_rms[2]){
      // Before checking if the Wumpus is hunted by the arrow, check if the arrow comes to
      // the player's own room. In that case, the player is killed by his own arrow, according
      // to Wikipedia
      if(cmd.room_nums[i]==player_rn){
	cout << "\tThe arrow came to your own Room, and you are killed by your own arrow...\n";
	player_rn = -1;
	return false;
      }
      HUNTED_WUMPUS = cave.shoot_a_room(cmd.room_nums[i]);
      current_rn = cmd.room_nums[i];
      shot_rms[i] = current_rn;
      if(HUNTED_WUMPUS)
	break;
    }
    else{
      // If the arrow destination is not a valid adjacent room #, set a random yet valid
      // (adjacent) room #
      current_rn = adj_rms[rand_int(0,2)];

      if(current_rn==player_rn){
	cout << "\tThe arrow came to your own Room, and you are killed by your own arrow...\n";
	player_rn = -1;
	return false;
      }
      
      HUNTED_WUMPUS = cave.shoot_a_room(current_rn);
      shot_rms[i] = current_rn;
      if(HUNTED_WUMPUS)
	break;
    }
  }

  // print which room(s) are shot
  cout << "\tRoom ";
  for(int i=0; i<3; ++i){
    if(shot_rms[i]==-1){
      // Since shot_rms[0] cannot be -1, i coming this if-sentence is either 1 or 2
      if(i==1) cout << " is";
      else cout << " are";	// if i==2
      break;
    }
    if(i==0) cout << shot_rms[i];
    else if(i==1) cout << ", " << shot_rms[i];
    else cout << ", and " << shot_rms[i] << " are";
  }
  cout << " shot by the magical arrow\n";
  
  return HUNTED_WUMPUS;
}

void HW_game::play(bool is_debug){
  // First, initialize the cave condition
  player_rn = cave.clear();
  // This process is redundant if this HW_game::play() is called for the first time after
  // the HW_game instance is made (because the initialization of the cave is already done
  // in the beginning of the HW_game's constructor)
  
  bool WUMPUS_HUNTED{false};
  // becomes true when the Wumpus is hunted

  int num_arrows{5};
  // The number of arrow the player can shoot is not mentioned either on the book or on
  // Wikipedia. But to make the game a little realistic and fun, I set a maximum number
  // of arrows. If the player runs out of all arrows, he loses.
  
  cout << "For the map of the cave, please see https://en.wikipedia.org/wiki/Hunt_the_Wumpus#/media/File:Hunt_the_Wumpus_map.svg\n";
  
  Command cmd;			// Command class is defined above
  int adj_rms[3];
  while(true){
    // first print where the player is, regardless of whether it is debug mode or not
    cout << "The player is at Room " << player_rn << ", and you have " << num_arrows
	 << " arrows." <<endl;
    cave.adjacent_rooms(player_rn, adj_rms);
    cout << "Tunnels lead to Room " << adj_rms[0] << ", " << adj_rms[1]
	 << ", and " << adj_rms[2] << endl;
    
    if(is_debug)
      cave.print();		// print where all the other entities are in the cave

    cave.notify_dangers(player_rn);

    // keep reading a command until the player input a valid command
    while(true){
      cout << "Move or shoot? ";
      if(cin >> cmd && is_valid_move(cmd))		// e.g. s13-4-3
	// here also checks if the destination of move 'm' is a valid neighbor, if cmd.move is
	// 'm'.
	// I don't care if in shoot 's', the destination rooms are valid or not, because
	// if they are invalid, the game picks up a valid option at random, according to
	// the wikipedia of this game (https://en.wikipedia.org/wiki/Hunt_the_Wumpus)
	break;
      else{
	cout << "An invalid command is entered. Examples are: m12, s12-3-4, s1-2\n";
	cout << "If command 'm' is entered, valid options are Room " << adj_rms[0] << ", "
	     << adj_rms[1] << ", " << adj_rms[2] << endl;;
	cout << "Enter another command again\n";
	cin.clear();
	// reset the fail state. Without this, cin >> cmd fails again without reading a new
	// command, which results in an infinite loop where those error outputs are infinitely
	// written.
      }
    }

    if(cmd.move == 'm'){
      // In the input process above, I already checked if the destination of the move 'm' is
      // valid in is_valid_move(cmd)
      player_rn = cave.move_player(cmd.room_nums[0]);
    }
    else{			// cmd is shoot 's'
      WUMPUS_HUNTED = shoot_rooms(cmd);
      // According to Wikipedia's gameplay explanation, every time after an arrow is shot,
      // the Wumpus is startled, and moves to an adjacent room

      num_arrows--;
      
      int wumpus_rn;
      wumpus_rn = cave.wumpus_startled(false);

      // As a result of the Wumpus moving after the shoot, if the Wumpus comes to the player's
      // Room, the player is killed
      if(wumpus_rn == player_rn){
	cout << "\tOuch! Your arrow startled the Wumpus, and it has entered your room...\n";
	player_rn = -1;
      }
    }

    if(player_rn < 0){
      // This means the player is dead, either by a pit or the Wunpus
      cout << "\tYou are dead!\n";
      break;
    }

    if(WUMPUS_HUNTED){
      cout << "\tCongratulation! You successfully hunted the Wumpus!\n";
      break;
    }

    // The checking of the number of arrows should come after the checking of whether the Wumpus
    // was hunted or not, because even when the number of arrows becomes 0, it's possible that
    // the last arrow hunted the Wumpus.
    if(num_arrows<1){
      cout << "\tYou ran out of arrows. There is no hope to hunt the Wumpus anymore...\n";
      cout << "\tYou lost.\n";
      break;
    }

    cout << endl;
  }
}

int main()
  try{
     
    HW_game hwgame;
    hwgame.run();
    
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
