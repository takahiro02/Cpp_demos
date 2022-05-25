

#include "Hunt_the_Wumpus.h"

namespace Hunt_the_Wumpus{
  

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
      // doesn't do range check (check if r is in [1,20])
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


Cave::Cave(ostream& ost)
  : os{ost}
{
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
      os << "\t I hear a bat\n";

    if(adj_rn == pit_rn[0] || adj_rn == pit_rn[1])
      os << "\t I feel a breeze\n";

    if(adj_rn == wumpus_rn)
      os << "\t I smell the wumpus\n";
  }
}

// print the state of the Cave
void Cave::print(ostream& ost){
  // I gave up printing the structure of the cave, since it's too much work, and little
  // return
  // Instead, I will just print where the bats, pits, and Wumpus are
  // The player's position is always printed at each turn.
  ost << "The bottomless pits are at Room " << pit_rn[0] << " and " << pit_rn[1] << endl;
  ost << "The giant bats are at Room " << bat_rn[0] << " and " << bat_rn[1] << endl;
  ost << "The Wumpus is at Room " << wumpus_rn << endl;
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

  bool Cave::move_player(int move_dest, int &ply_rn){
    // The caller, HW_game, also has player_rn as a member variable. To update it as well as
    // pllayer_rn in Cave class, this function receives HW_game::player_rn as reference.
    // (I think I can manage Cave class without player_rn, but for now, I will go with the
    // 2 player_rn s in each of Cave and HW_game
    
  if(move_dest < 1 || move_dest > 20)
    error("Error in Cave::move_player(), invalid destination is used");

  // Whether or not player is killed, player_rn s store the moved destination, to refer to
  // the last Room if the player is killed by either the Wumpus ot a pit,
  // for HW_game_window::ready_pressed()
  player_rn = move_dest;
  ply_rn = player_rn;
  
  // First, check if there is a pit in the destination, because if a pit exists, that results
  // in the player's death instantly
  // (Even if the Wumpus is in the room, it may move to an adjacent room witn 50 % chance, so
  //  So the Wumpus existing in the destination room doesn't necessarily mean the player's
  //  death)
  if(move_dest == pit_rn[0] || move_dest == pit_rn[1]){
    os << "\tThe player fell into a pit!\n";
    return true;		// return value is stored in HW_game::PLAYER_KILLED
  }

  // Second, check the Wumpus, bacause even if a bat and the Wumpus co-exist in the destination
  // room, if the Wumpus happens to stay in the room, the player is killed, which supercedes
  // a bat's replacement of the player
  if(move_dest == wumpus_rn){
    wumpus_rn = wumpus_startled();
    // if, even after the Wumpus is startled by the player, it stays in the same room, the
    // player is killed by it
    if(wumpus_rn == move_dest){
      os << "\tThe Wumpus is in the room!\n";
      return true;
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
    os << "\tA giant bat has carried you to Room " << new_dest << "!\n";
   
    // If in the new destination, a pit and/or the Wumpus exist, we have to repeat the same
    // checking process for the new destination. So move_player() is recursively called here.
    return move_player(new_dest, ply_rn);;
  }

  // reaching here means in the destination Room, no threats exist. So player is still alive
  // == HW_game::PLAYER_KILLED stays false
  return false;
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

void HW_game::run(){
  char r;
  char m;
  string buff;		// buffer
  while(true){
    os << "Do you play Hunt the Wumpus? (y/n) ";
    is >> r;
    if(r == 'y'){
      os << "In debug mode, or normal mode? (d/n) ";
      is >> m;
      if(m == 'd'){
	getline(is, buff);
	// Without getline(), the first read of Command in play() failed. I don't exactly know
	// why
	this->play(true);	// "this->" is not necessary, but for clarification
      }
      else if(m == 'n'){
	getline(is, buff);
	this->play(false);
      }
      else
	os << "Unknown mode is entered. Mode is 'd' or 'n'.\n";
    }
    else if(r == 'n')
      break;
    else
      os << "Sorry, can you answer either 'y' or 'n'?\n";
  }
}

bool HW_game::is_valid_move(Command cmd){
  // If the move is shoot, always return true regardless of the destination of the shoot
  if(cmd.move == 's')
    return true;
  
  int adj_rms[3];
  cave.adjacent_rooms(player_rn, adj_rms);
  // This player_rn is the position before this cmd is reflected into the actual move. So
  // player_rn is in the valid range [1,20] at this point
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
	os << "\tThe arrow came to your own Room, and you are killed by your own arrow...\n";
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
	os << "\tThe arrow came to your own Room, and you are killed by your own arrow...\n";
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
  os << "\tRoom ";
  for(int i=0; i<3; ++i){
    if(shot_rms[i]==-1){
      // Since shot_rms[0] cannot be -1, i coming this if-sentence is either 1 or 2
      if(i==1) os << " is";
      else os << " are";	// if i==2
      break;
    }
    if(i==0) os << shot_rms[i];
    else if(i==1) os << ", " << shot_rms[i];
    else os << ", and " << shot_rms[i] << " are";
  }
  os << " shot by the magical arrow\n";
  
  return HUNTED_WUMPUS;
}

void HW_game::play(bool is_debug){
  // First, initialize the cave condition
  player_rn = cave.clear();
  // This process is redundant if this HW_game::play() is called for the first time after
  // the HW_game instance is made (because the initialization of the cave is already done
  // in the beginning of the HW_game's constructor)

  WUMPUS_HUNTED = false;
  num_arrows = 5;
  STILL_IN_GAME = true;
  
  os << "For the map of the cave, please see https://en.wikipedia.org/wiki/Hunt_the_Wumpus#/media/File:Hunt_the_Wumpus_map.svg\n";

  Command cmd;			// Command class is defined above
  int adj_rms[3];
  
  while(true){
    // first print where the player is, regardless of whether it is debug mode or not
    os << "The player is at Room " << player_rn << ", and you have " << num_arrows
       << " arrows." <<endl;
    cave.adjacent_rooms(player_rn, adj_rms);
    os << "Tunnels lead to Room " << adj_rms[0] << ", " << adj_rms[1]
       << ", and " << adj_rms[2] << endl;

    if(is_debug)
      cave.print();		// print where all the other entities are in the cave
    
    cave.notify_dangers(player_rn);

    // keep reading a command until the player input a valid command
    while(true){
      os << "Move or shoot? ";
      if(get_command(cmd))
	break;
      else{
	os << "An invalid command is entered. Examples are: m12, s12-3-4, s1-2\n";
	os << "If command 'm' is entered, valid options are Room " << adj_rms[0] << ", "
	     << adj_rms[1] << ", " << adj_rms[2] << endl;;
	os << "Enter another command again\n";
	is.clear();
	// reset the fail state. Without this, is >> cmd fails again without reading a new
	// command, which results in an infinite loop where those error outputs are infinitely
	// written.
      }
    }

    move_1turn(cmd);
    os << endl;
    // After this 1 turn ends, for readability, put a newline 

    if(!STILL_IN_GAME)
      break;
  }
}

  bool HW_game::get_command(Command& cmd){
      if(is >> cmd && is_valid_move(cmd))		// e.g. s13-4-3
	// here also checks if the destination of move 'm' is a valid neighbor, if cmd.move is
	// 'm'.
	// I don't care if in shoot 's', the destination rooms are valid or not, because
	// if they are invalid, the game picks up a valid option at random, according to
	// the wikipedia of this game (https://en.wikipedia.org/wiki/Hunt_the_Wumpus)
	return true;
      else
	return false;
  }

  void HW_game::move_1turn(Command cmd){
    // Command argument is assumed to be valid, with using get_command(Command&) function

    if(cmd.move == 'm'){
      // In the input process above, I already checked if the destination of the move 'm' is
      // valid in is_valid_move(cmd)
      PLAYER_KILLED = cave.move_player(cmd.room_nums[0], player_rn);
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
	os << "\tOuch! Your arrow startled the Wumpus, and it has entered your room...\n";
	PLAYER_KILLED = true;
      }
    }

    if(PLAYER_KILLED){
      // This means the player is dead, either by a pit or the Wunpus
      os << "\tYou are dead!\n";
      STILL_IN_GAME = false;
      return;
    }

    if(WUMPUS_HUNTED){
      os << "\tCongratulation! You successfully hunted the Wumpus!\n";
      STILL_IN_GAME = false;
      return;
    }

    // The checking of the number of arrows should come after the checking of whether the Wumpus
    // was hunted or not, because even when the number of arrows becomes 0, it's possible that
    // the last arrow hunted the Wumpus.
    if(num_arrows<1){
      os << "\tYou ran out of arrows. There is no hope to hunt the Wumpus anymore...\n";
      os << "\tYou lost.\n";
      STILL_IN_GAME = false;
      return;
    }
  }

  // =========================================================================================
  
  HW_game_window::HW_game_window(Point tl, int w, int h, const string& title)
    : Window{tl, w, h, title}, HW_game{iss, oss},
    quit{Point{x_max()-70,0}, 70, 20, "Quit",
	 [](Address, Address wp){reference_to<HW_game_window>(wp).quit_pressed();}},
    ready{Point{280,y_max()-180}, 70, 20, "Go",
	  [](Address, Address wp){reference_to<HW_game_window>(wp).ready_pressed();}},
      command_box{Point{80,y_max()-180}, 180, 30, "command:"},
      debug{Point{0,y_max()-25}, 100, 20, "Debug on/off",
	    [](Address, Address wp){reference_to<HW_game_window>(wp).debug_pressed();}},
      reset{Point{150,y_max()-25}, 80, 20, "Reset",
	    [](Address, Address wp){reference_to<HW_game_window>(wp).reset_pressed();}},
      result_box{Point{80,y_max()-130}, 300, 100, "result:"},
      debug_box{Point{460,y_max()-130}, 300, 100, "debug info:"},
      is_debug{false},
      cave_map{Point{580, 220}, 200},
      
      mark_box{Point{80,y_max()/2}, 150, 30, "Mark"},
      room_num_box{Point{80,y_max()/2-35}, 70, 30, "Room #"},
      put{Point{170,y_max()/2-30}, 70, 20, "Put",
	  [](Address, Address wp){reference_to<HW_game_window>(wp).put_pressed();}}
  {
    attach(quit);
    attach(ready);
    attach(debug);
    attach(reset);
    attach(command_box);
    attach(result_box);
    attach(debug_box);
    attach(cave_map);

    // Ex 15 ch19
    attach(mark_box);
    attach(room_num_box);
    attach(put);

    int adj_rms[3];
    oss << "The player is at Room " << player_rn << ", \n and you have " << num_arrows
	<< " arrows." <<endl;
    cave.adjacent_rooms(player_rn, adj_rms);
    oss << "Tunnels lead to Room " << adj_rms[0] << ", " << adj_rms[1]
	<< ", and " << adj_rms[2] << endl;

    cave.notify_dangers(player_rn);

    // show the player's location on the cave map
    cave_map.visit(player_rn);

    string s{oss.str()};
    result_box.put(s);
    oss.str("");

    // Since bats and pits don't move, I can get these locations at this constructor and keep them
    cave.get_bat_rooms(bat_rms);
    cave.get_pit_rooms(pit_rms);
  }

  void HW_game_window::ready_pressed(){
    string si{command_box.get_string()};
    si += '\n';
    // Without appending newline '\n' at the end, getline(is, line); in
    // istream& operator>>(istream& is, Command &cmd) sets the state of is to fail state after
    // reading is.
    iss.str(si); // set read command string

    int adj_rms[3];

    if(!STILL_IN_GAME){
      oss << "The game was already finished.\n";
      oss << "If you want to play the game again, \n  press reset button below\n";
    }
    else if(STILL_IN_GAME && !this->get_command(cmd)){
      // if an invalid command is entered
      // After the player is dead == player_rn is -1, when this window tries to read the next
      // command, it causes error in adjacent_rooms() in is_valid_move() in get_command(), and
      // the window is closed.
      
      this->cave.adjacent_rooms(player_rn, adj_rms);
      // cave is one of the base classes, HW_game's protected member. this-> is not necessary,
      // this-> is necessary only when this class (or the base class) is template classes
      
      oss << "An invalid command is entered.\nExamples are: m12, s12-3-4, s1-2\n";
      oss << "If command 'm' is entered,\n valid options are Room " << adj_rms[0] << ", "
	 << adj_rms[1] << ", " << adj_rms[2] << endl;;
      oss << "Enter another command again\n";
      iss.clear();

      if(is_debug) cave.print(oss_debug); // print where all the other entities are in the cave
    }
    else{			// a valid command is entered
      // get_command() also checks if the moving destination is a correct neighbor, if the
      // command is 'm' (move).

      if(cmd.move == 'm'){
	// if the moved destination has a danger of Wumpus, a Bat, or a pit, the moved 
	// destination is not marked as "visited" in cave_map, because once the player come 
	// across one of such dangers, player_rn becomes -1 (if the danger is the Wumpus or a 
	// pit), or a random Cave room number if the danger is a bat, before the visited room 
	// with the danger is marked as "VISITED".
	// To mark the rooms with a danger as VISITED, I added this extra step before
	// move_1turn().
	cave.get_wumpus_room(wumpus_rm);
	// The locations of bats and pits were already taken in HW_game_window()'s constructor,
	// and HW_game_window::reset_pressed(), since they are fixed thoroughout 1 game

	if(cmd.room_nums[0] == bat_rms[0] || cmd.room_nums[0] == bat_rms[1] ||
	   cmd.room_nums[0] == pit_rms[0] || cmd.room_nums[0] == pit_rms[1] ||
	   cmd.room_nums[0] == wumpus_rm){
	  cave_map.visit(cmd.room_nums[0], false, true);
	  // This sets previous_player_rn to cmd.room_nums[0] in cave_map.visit()
	  redraw();
	}
      }
      
      this->move_1turn(cmd);
      // strings are put into oss passed in initializing hw_game in the constructor
      // HW_game_window()

      // deal with the case where a giant bat carries the player to a deadly Room, and player
      // died. Without this, the cave_map shows only the bat's Room in the end of the game.
      // I also want to show the last Room that killed the player
      cave.get_wumpus_room(wumpus_rm); // wumpus's room can change in HW_game::move_1turn(),
      // so update wumpus_rm. There are 3 patterns of updating the Wumpus's room:
      // 1: player shoots an arrow, and that startled the Wumpus, moved it to an adjacent room,
      //    and if player was in the room, he was killed.
      // 2: player shoots an arrow, and that startled the Wumpus, moved it to an adjacent room,
      //    and player wasn't in the adjacent room. So he is still alive.
      // 3: player moved into the Wumpus's room, which startled it, and moved it to and
      //    adjacent room. So player is still alive.
      // I omit cases where the Wumpus is startled, but stayed in the same room. None of the
      // above 3 patterns are related to the case I want to deal with in the following if-clause.
      // The case I want to deal with is when the Wumpus chose to stay in the same room.
      // But updating wumpus_rm doesn't hurt. So let's just reflect the latest Wumpus location
      // to the code.
      if(player_rn == pit_rms[0] || player_rn == pit_rms[1] || player_rn == wumpus_rm){
	// player_rn was updated in move_1turn(). I modified the code of HW_game::move_1turn() 
	// and Cave::move_player() so that even when the player is dead, player_rn holds the
	// last Room he was in, instead of holding -1. I introduced another boolean variable
	// "PLAYER_KILLED" to let HW_game class know whether player was killed, instead.
	cave_map.visit(player_rn, false, true);
	// This sets previous_player_rn to cmd.room_nums[0] in cave_map.visit()
	redraw();
      }      
      
      // It's possible that at this point, the Wumpus kills the player, and the player_rn is
      // -1. If so, without this if-check with STILL_IN_GAME, in cave.notify_dangers(player_rn),
      // it causes invalid room number error.
      if(STILL_IN_GAME){
	// show the location on the map
	cave_map.visit(player_rn);
	redraw();			// calling cave_map.draw(); directly doesn't work
	
	if(is_debug) cave.print(oss_debug); // print where all the other entities are in the cave

	cave.notify_dangers(player_rn);
      }
    }

    string s{oss.str()};	// we need to prepare another string variable, because
    // Out_box::put(const string&) takes a (const) reference to a string
    result_box.put(s);
    oss.str("");		// set empty string
    oss.clear();		// clear any possible error flags (may be redundant)

    // output to debug box
    string s2{oss_debug.str()};
    debug_box.put(s2);
    oss_debug.str("");
    oss_debug.clear();
  }
  // Since 1 push of the command input button is supposed to move the game forward by 1 turn,
  // we cannot use HW_game::play(), which deals with the whole game play. Instead, I imitate
  // the logical flow of HW_game::play() in this ready_pressed() function

  void HW_game_window::reset_pressed(){
    // Initialize the game conditions
    // The following code is copied from the 1st part of HW_game::play().
    // The following variables are inherited members of HW_game class
    player_rn = cave.clear();
    WUMPUS_HUNTED = false;
    num_arrows = 5;
    STILL_IN_GAME = true;

    // initialize the cave map too
    cave_map.clear();
    cave_map.visit(player_rn);
    redraw();
    
    // After initializing the game, print initial information (copied from HW_game_window's
    // constructor)
    int adj_rms[3];
    oss << "The player is at Room " << player_rn << ", \n and you have " << num_arrows
	<< " arrows." <<endl;
    cave.adjacent_rooms(player_rn, adj_rms);
    oss << "Tunnels lead to Room " << adj_rms[0] << ", " << adj_rms[1]
	<< ", and " << adj_rms[2] << endl;
    
    if(is_debug) cave.print(oss_debug);	// print where all the other entities are in the cave
    
    cave.notify_dangers(player_rn);

    string s{oss.str()};	// we need to prepare another string variable, because
    // Out_box::put(const string&) takes a (const) reference to a string
    result_box.put(s);
    oss.str("");		// set empty string
    oss.clear();		// clear any possible error flags (may be redundant)

    // output to debug box
    string s2{oss_debug.str()};
    debug_box.put(s2);
    oss_debug.str("");
    oss_debug.clear();

    // These lines are for marking these rooms with a danger as VISITED
    cave.get_bat_rooms(bat_rms);
    cave.get_pit_rooms(pit_rms);
  }

  // Ex 15 ch19
  void HW_game_window::put_pressed(){
    int room_num{room_num_box.get_int()};
    string mark{mark_box.get_string()};

    // if the input room # is invalid, do nothing
    if(room_num < 1 || room_num > 20)
      return;
    mark = to_string(room_num) + " " + mark;
    cave_map.set_label(room_num, mark);
    redraw();
  }
  
  // =========================================================================================

  Cave_map::Cave_map(Point center_point, int radius)
    : font_size{15}, player_rn{-1}, previous_player_rn{-1}
  {
    int room_radius{radius/10};	// radius of each cave room (each Texted_circle)

    room_vec.push_back(new Room{Point{0,0}, 100}); // fill in a dummy Room object for the 1st
    // object, to make the indexing of room_vec start from 1, instead of 0
    
    Point room_center;
    // the outermost layer
    for(int i=0; i<5; ++i){
      room_center.x = center_point.x + (radius-room_radius)*cos(deg2rad(-90+i*(360/5.0)));
      room_center.y = center_point.y + (radius-room_radius)*sin(deg2rad(-90+i*(360/5.0)));
      room_vec.push_back(new Room{room_center, room_radius});
      room_vec[room_vec.size()-1].tc.set_label(to_string(i+1));
      room_vec[room_vec.size()-1].tc.set_font_size(font_size);
    }
    // make passages (Line) between Rooms
    tuple<double, double> line_form;		// line: y = a*x + b
    char which[2]{'b', 's'};			// there are 2 intersections in the line and
    // a circle. This array gives which of them to be chosen for each pair of 2 circles
    // 'b' means to choose bigger one in x coordinate, 's' means smaller.
    // As I wrote in my note, this pattern alternately appears
    for(int i=1; i<room_vec.size(); ++i){
      Point p1, p2;		// intersections of the line connecting 2 Texted_circles' centers
      // and each of the 2 Texted_circles
      p1 = room_vec[i].tc.center();
      int i2 = (i+1==room_vec.size())? 1 : i+1;
      p2 = room_vec[i2].tc.center();

      line_form = get_line_formula(p1, p2);

      Point intersec1, intersec2;
      intersec1 = get_intersec(p1, room_radius, get<0>(line_form), get<1>(line_form), which[0]);
      intersec2 = get_intersec(p2, room_radius, get<0>(line_form), get<1>(line_form), which[1]);
      
      line_vec.push_back(new Line{intersec1, intersec2});

      // set pointers of the 2 sides of this Line for this added Line
      // room_vec[i].l[0] = &line_vec[line_vec.size()-1];
      // room_vec[i2].l[1] = &line_vec[line_vec.size()-1];
      room_vec[i].passages[0] = Passage{&line_vec[line_vec.size()-1], i2};
      room_vec[i2].passages[1] = Passage{&line_vec[line_vec.size()-1], i};
      
      // b-s pair doesn't necessarily alternate. There is no fixed pattern. See my note
      if(i!=2 && i!=3){
	which[0] = (which[0]=='b')? 's' : 'b';
	which[1] = (which[1]=='b')? 's' : 'b';
      }
    }
    // // let the pointers point to the corresponding Line objects
    // room_vec[1].l[0] = &line_vec[0], room_vec[1].l[1] = &line_vec[line_vec.size()-1];
    // // Only room_vec[1] differs from the others, in that the adjacent Line indices jumps
    // // from line_vec[0] to line_vec[4]
    // for(int i=2; i<room_vec.size(); ++i){
    //   room_vec[i].l[0] = &line_vec[i-1], room_vec[i].l[1] = &line_vec[line_vec.size()-i];
    // }
    // <- This way of assigning Line objects to pointer l after these Lines are added is
    //    error prone, because we have to take care of indexing of line_vec as well as indexing
    //    of room_vec in this way. Rather, assigning Line object to pointer l as soon as the
    //    Line object is added to line_vec is better, because in that way, we only need to care
    //    for the indexing of room_vec as abovve

    // the middle layer
    for(int i=0; i<10; ++i){
      room_center.x = center_point.x + (2.0/3)*(radius-room_radius)*cos(deg2rad(-90-36*2+i*(360/10.0)));
      room_center.y = center_point.y + (2.0/3)*(radius-room_radius)*sin(deg2rad(-90-36*2+i*(360/10.0)));
      room_vec.push_back(new Room{room_center, room_radius});
      room_vec[room_vec.size()-1].tc.set_label(to_string(room_vec.size()-1));
      room_vec[room_vec.size()-1].tc.set_font_size(font_size);
    }
    //make passages (Line) between Rooms
    which[0] = 'b', which[1] = 's';
    for(int i=6; i<room_vec.size(); ++i){
      Point p1, p2;		// intersections of the line connecting 2 Texted_circles' centers
      // and each of the 2 Texted_circles
      p1 = room_vec[i].tc.center();
      int i2 = (i+1==room_vec.size())? 6 : i+1;
      // p2 = (i+1==room_vec.size())? room_vec[6].tc.center() : room_vec[i+1].tc.center();
      p2 = room_vec[i2].tc.center();

      	Point intersec1, intersec2;
      if(p1.x != p2.x){
	line_form = get_line_formula(p1, p2);

	intersec1 = get_intersec(p1, room_radius, get<0>(line_form), get<1>(line_form), which[0]);
	intersec2 = get_intersec(p2, room_radius, get<0>(line_form), get<1>(line_form), which[1]);
      }
      else{			// if p1.x == p2.x
	// between Room 10 and 11, and Room 15 and 6, the line connecting 2 Circle's centers
	// becomes the form of x=value. (i.e. can't be expressed in the form of y=a*x + b).
	// So we need to separate these cases from the others.
	int sign[2]{+1, -1};	// which intersection to choose changes with index i
	if(i+1==room_vec.size()){
	  sign[0] = -1, sign[1] = +1;
	}
	intersec1.x = p1.x;	// == p2.x
	intersec2.x = p1.x;
	intersec1.y = p1.y + sign[0]*room_radius;
	int i2{i+1};
	if(i+1==room_vec.size()) // for when i is 15
	  i2 = 6;
	intersec2.y = p2.y + sign[1]*room_radius;
      }
      
      line_vec.push_back(new Line{intersec1, intersec2});

      // set pointers of the 2 sides of this Line for this added Line
      // room_vec[i].l[0] = &line_vec[line_vec.size()-1];
      // room_vec[i2].l[1] = &line_vec[line_vec.size()-1];
      room_vec[i].passages[0] = Passage{&line_vec[line_vec.size()-1], i2};
      room_vec[i2].passages[1] = Passage{&line_vec[line_vec.size()-1], i};

      // b-s pair doesn't necessarily alternate. There is no fixed pattern. See my note
      if(i==10){
    	which[0] = (which[0]=='b')? 's' : 'b';
    	which[1] = (which[1]=='b')? 's' : 'b';
      }
    }
    // // let the pointers point to the corresponding Line objects
    // room_vec[6].l[0] = &line_vec[5], room_vec[6].l[1] = &line_vec[14];
    // for(int i=7; i<room_vec.size(); ++i)
    //   room_vec[i].l[0] = &line_vec[i-1], room_vec[i].l[1] = &line_vec[i-2];

    // Passages between the outermost circle and middle circle
    int increment{0};
    // connected Rooms are 1-8 (diff:7), 2-10 (diff:8), 3-12 (diff:9), 4-14 (diff:10), so the
    // diff increments. This "increment" variable is used to increment the diff
    which[0] = 's', which[1] = 'b';
    Point p1, p2;		// intersections of the line connecting 2 Texted_circles' centers
    for(int i=1; i<5; ++i){
      // and each of the 2 Texted_circles
      p1 = room_vec[i].tc.center();
      p2 = room_vec[i+7+increment].tc.center();

      Point intersec1, intersec2;
      if(p1.x != p2.x){
	line_form = get_line_formula(p1, p2);

	intersec1 = get_intersec(p1, room_radius, get<0>(line_form), get<1>(line_form), which[0]);
	intersec2 = get_intersec(p2, room_radius, get<0>(line_form), get<1>(line_form), which[1]);
      }
      else{
	// between Room 1 and 8,  the line connecting 2 Circle's centers
	// becomes the form of x=value. (i.e. can't be expressed in the form of y=a*x + b).
	intersec1.x = p1.x;	// == p2.x
	intersec2.x = p1.x;
	intersec1.y = p1.y + room_radius;
	intersec2.y = p2.y - room_radius;
      }
      line_vec.push_back(new Line{intersec1, intersec2});

      // set pointers of the 2 sides of this Line for this added Line
      // Passages between different layers are stored in l[2]
      //room_vec[i].l[2] = &line_vec[line_vec.size()-1];
      //room_vec[i+7+increment].l[2] = &line_vec[line_vec.size()-1];
      // I once made the mistake that I set the indices of l here to l[0] or l[1].
      // If that happens, l[2] of these rooms stays nullptr, and if so, in Cave_map::visit(),
      // in the line of setting the color of passages, that is translated as
      // nullptr->set_color(Color::black);, which leads to segmentation fault.
      room_vec[i].passages[2] = Passage{&line_vec[line_vec.size()-1], i+7+increment};
      room_vec[i+7+increment].passages[2] = Passage{&line_vec[line_vec.size()-1], i};
      
      // b-s pair doesn't necessarily alternate. There is no fixed pattern. See my note
      if(i==3){
	which[0] = (which[0]=='b')? 's' : 'b';
	which[1] = (which[1]=='b')? 's' : 'b';
      }
      ++increment;
    }
    // For the passage between Room 5 and 6, since the diff is different from the above 4
    // passages, I separate it from the for loop above
    p1 = room_vec[5].tc.center();
    p2 = room_vec[6].tc.center();
    Point intersec1, intersec2;
    line_form = get_line_formula(p1, p2);
    which[0] = 'b', which[1] = 's';
    intersec1 = get_intersec(p1, room_radius, get<0>(line_form), get<1>(line_form), which[0]);
    intersec2 = get_intersec(p2, room_radius, get<0>(line_form), get<1>(line_form), which[1]);
    line_vec.push_back(new Line{intersec1, intersec2});
    // set pointers for this added Line
    // room_vec[5].l[2] = &line_vec[line_vec.size()-1];
    // room_vec[6].l[2] = &line_vec[line_vec.size()-1];
    room_vec[5].passages[2] = Passage{&line_vec[line_vec.size()-1], 6};
    room_vec[6].passages[2] = Passage{&line_vec[line_vec.size()-1], 5};

    // let the pointers point to the corresponding Line objects that are created just above
    // room_vec[5].l[2] = &line_vec[line_vec.size()-1];
    // room_vec[6].l[2] = &line_vec[line_vec.size()-1];
    // increment = 0;
    // for(int i=1; i<5; ++i){
    //   room_vec[i].l[2] = &line_vec[line_vec.size()-i-1];
    //   room_vec[i+increment+7].l[2] = &line_vec[line_vec.size()-i-1];
    //   ++increment;
    // }

    // The innermost layer
    for(int i=0; i<5; ++i){
      room_center.x = center_point.x + (1.0/3)*(radius-room_radius)*cos(deg2rad(90+72+i*(360/5.0)));
      room_center.y = center_point.y + (1.0/3)*(radius-room_radius)*sin(deg2rad(90+72+i*(360/5.0)));
      room_vec.push_back(new Room{room_center, room_radius});
      room_vec[room_vec.size()-1].tc.set_label(to_string(room_vec.size()-1));
      room_vec[room_vec.size()-1].tc.set_font_size(font_size);
    }
    // make passages (Line) between Rooms
    for(int i=16; i<room_vec.size(); ++i){
      p1 = room_vec[i].tc.center();
      int i2 = (i+1==room_vec.size())? 16 : i+1;
      //p2 = (i+1==room_vec.size())? room_vec[16].tc.center() : room_vec[i+1].tc.center();
      p2 = room_vec[i2].tc.center();
      
      line_form = get_line_formula(p1, p2);

      intersec1 = get_intersec(p1, room_radius, get<0>(line_form), get<1>(line_form), which[0]);
      intersec2 = get_intersec(p2, room_radius, get<0>(line_form), get<1>(line_form), which[1]);
      line_vec.push_back(new Line{intersec1, intersec2});

      // set pointers for this added Line
      // room_vec[i].l[0] = &line_vec[line_vec.size()-1];
      // room_vec[i2].l[1] = &line_vec[line_vec.size()-1];
      room_vec[i].passages[0] = Passage{&line_vec[line_vec.size()-1], i2};
      room_vec[i2].passages[1] = Passage{&line_vec[line_vec.size()-1], i};
      
      // b-s pair doesn't necessarily alternate. There is no fixed pattern. See my note
      if(i==18){
	which[0] = (which[0]=='b')? 's' : 'b';
	which[1] = (which[1]=='b')? 's' : 'b';
      }
    }
    // let the pointers point to the corresponding Line objects
    // Only room_vec[16] has adjacent Lines whose indices in line_vec are not contiguous
    // room_vec[16].l[0] = &line_vec[20];
    // room_vec[16].l[1] = &line_vec[24];
    // increment = 0;
    // for(int i=17; i<room_vec.size(); ++i){
    //   room_vec[i].l[0] = &line_vec[21+increment];
    //   room_vec[i].l[1] = &line_vec[20+increment];
    //   ++increment;
    // }

    // Passages between the middle circle and innermost circle
    // connected Rooms are 17-7 (diff:10), 18-9 (diff:9), 19-11 (diff:8), 20-13 (diff:7), so the
    // diff decrements.
    increment = 0;
    which[0] = 's', which[1] = 'b';
    for(int i=17; i<room_vec.size(); ++i){
      p1 = room_vec[i].tc.center();
      p2 = room_vec[i-10+increment].tc.center();

      if(p1.x != p2.x){
	line_form = get_line_formula(p1, p2);

	intersec1 = get_intersec(p1, room_radius, get<0>(line_form), get<1>(line_form), which[0]);
	intersec2 = get_intersec(p2, room_radius, get<0>(line_form), get<1>(line_form), which[1]);
      }
      else{
	// between Room 20 and 13,  the line connecting 2 Circle's centers becomes
	// the form of x=value (vertical). (i.e. can't be expressed in the form of y=a*x + b).
	intersec1.x = p1.x;	// == p2.x
	intersec2.x = p1.x;
	intersec1.y = p1.y + room_radius;
	intersec2.y = p2.y - room_radius;
      }
      line_vec.push_back(new Line{intersec1, intersec2});
      // b-s pair doesn't necessarily alternate. There is no fixed pattern. See my note

      // set pointers for this added Line
      // room_vec[i].l[2] = &line_vec[line_vec.size()-1];
      // room_vec[i-10+increment].l[2] = &line_vec[line_vec.size()-1];
      room_vec[i].passages[2] = Passage{&line_vec[line_vec.size()-1], i-10+increment};
      room_vec[i-10+increment].passages[2] = Passage{&line_vec[line_vec.size()-1], i};
      
      if(i==17){
	which[0] = (which[0]=='b')? 's' : 'b';
	which[1] = (which[1]=='b')? 's' : 'b';
      }
      ++increment;
    }
    // For the passage between Room 15 and 16, since the diff is different from the above 4
    // passages, I separate it from the for loop above
    p1 = room_vec[15].tc.center();
    p2 = room_vec[16].tc.center();
    line_form = get_line_formula(p1, p2);
    which[0] = 'b', which[1] = 's';
    intersec1 = get_intersec(p1, room_radius, get<0>(line_form), get<1>(line_form), which[0]);
    intersec2 = get_intersec(p2, room_radius, get<0>(line_form), get<1>(line_form), which[1]);
    line_vec.push_back(new Line{intersec1, intersec2});
    // set pointers for this added Line
    // room_vec[15].l[2] = &line_vec[line_vec.size()-1];
    // room_vec[16].l[2] = &line_vec[line_vec.size()-1];
    room_vec[15].passages[2] = Passage{&line_vec[line_vec.size()-1], 16};
    room_vec[16].passages[2] = Passage{&line_vec[line_vec.size()-1], 15};
    
    // let the pointers point to the corresponding Line objects that are created just above
    // room_vec[15].l[2] = &line_vec[line_vec.size()-1];
    // room_vec[16].l[2] = &line_vec[line_vec.size()-1];
    // increment = 1;
    // for(int i=20; i>16; --i){
    //   room_vec[i].l[2] = &line_vec[line_vec.size()-increment-1];
    //   room_vec[i-7-(increment-1)].l[2] = &line_vec[line_vec.size()-increment-1];
    //   ++increment;
    // }

    // In the end, we make all Graph class objects invisible
    for(int i=1; i<room_vec.size(); ++i){
      room_vec[i].tc.set_color(Color::invisible);
      room_vec[i].tc.set_font_size(0);
    }
    for(int i=0; i<line_vec.size(); ++i)
      line_vec[i].set_color(Color::invisible);
  }

  // get the intersection between the line connecting 2 Texted_circles' centers and 1 of the
  // 2 Texted_circles. There are 2 intersections in the line and such a circle. We choose one
  // of them by specifying which of these we choose based on the x coordinate of the 2 points
  Point Cave_map::get_intersec(Point center_point, int radius, double a, double b, char which){
    // the line: y = a*x + b
    // which = either 's' (meaning smaller one in x coordinate) or 'b' (meaning bigger one)
    double x, y;
    
    double l, m, n;
    // intermediate coefficients of l*x^2 + 2*m*x + n = 0 for solving for the x coordinate of
    // the intersection
    l = 1+a*a;
    m = a*b - a*center_point.y - center_point.x;
    n = center_point.x*center_point.x + (b-center_point.y)*(b-center_point.y) - radius*radius;
    
    if(which == 'b'){
      x = (-m + sqrt(m*m - l*n)) / l;
    }
    else if(which == 's'){
      x = (-m - sqrt(m*m - l*n)) / l;
    }
    else
      throw runtime_error("Error in Cave_map::get_intersec_point(). 'which' must be either 'b' or 's'");

    y = a*x + b;
    return Point{static_cast<int>(x), static_cast<int>(y)};
  }

  void Cave_map::visit(int plr_rn, bool MAKE_PREV_INVISIBLE, bool MAKE_ADJ_INVISIBLE){
    // MAKE_PREV_INVISIBLE : default value is false.
    // With MAKE_PREV_INVISIBLE true, the passages and adjacent room numbers to the previously
    // visited Room that have not been visited yet are made invisible
    
    player_rn = plr_rn;
    if(previous_player_rn < 0){	// means this visit is the first call after resetting the Cave
      previous_player_rn = player_rn;
    }

    room_vec[previous_player_rn].tc.set_fill_color(Color::white);
    int adj_rn;			// adjacent room number
    if(MAKE_PREV_INVISIBLE){
      // make previously adjacent rooms invisible, if they have not been visited yet
      for(int i=0; i<3; ++i){
	adj_rn = room_vec[previous_player_rn].passages[i].leading_rn;
	if(room_vec[adj_rn].VISITED == false){
	  room_vec[adj_rn].tc.set_font_size(0);
	  room_vec[previous_player_rn].passages[i].l_ptr->set_color(Color::invisible);
	}
      }
    }
    previous_player_rn = player_rn;
    
    room_vec[player_rn].VISITED = true;
    room_vec[player_rn].tc.set_color(Color::black);
    room_vec[player_rn].tc.set_fill_color(Color::red);
    room_vec[player_rn].tc.set_font_size(font_size);
    // room_vec[player_rn].tc.set_font_color(Color::green);
    // have facility to manipulate the font color as well, but I will use default color black,
    // which is set by default in Text_circle's constructors

    if(!MAKE_ADJ_INVISIBLE)	// don't make adjacent rooms and passages visible if it is false
      for(int i=0; i<3; ++i){
	room_vec[player_rn].passages[i].l_ptr->set_color(Color::black);
	// At this line, if any of l[i] is still nullptr, that leads to be translated as
	// nullptr->set_color(Color::black);, which leads to segmentation fault.
	adj_rn = room_vec[player_rn].passages[i].leading_rn;
	room_vec[adj_rn].tc.set_font_size(font_size);
      }
  }

  // set all Colors invisible
  void Cave_map::clear(){
    for(int i=1; i<room_vec.size(); ++i){
      room_vec[i].tc.set_color(Color::invisible);
      room_vec[i].tc.set_fill_color(Color::invisible);
      room_vec[i].tc.set_font_size(0);
      // I modified Text_circle::draw_lines(), so that when the font size of the Text inside
      // is 0, the Text becomes invisible (the drawing step is skipped)
    }
    for(int i=0; i<line_vec.size(); ++i)
      line_vec[i].set_color(Color::invisible);
    player_rn = -1;
    previous_player_rn = -1;
  }
  
  void Cave_map::draw_lines() const {
    for(int i=1; i<room_vec.size(); ++i)
      // room_vec[0] is a dummy Room used for making the indexing start from 1, so skip i==0
      room_vec[i].tc.draw_lines(); // draw only Texted_circle
    
    for(int i=0; i<line_vec.size(); ++i)
      line_vec[i].draw_lines();
  }
  // =========================================================================================
};				// namespace Hunt_the_Wumpus {
