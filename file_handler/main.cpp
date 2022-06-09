

#include "std_lib_facilities.h"


// Since fstream already practices RAII, I use C-style file management (FILE*, fopen(), fclose())
struct File_handle {
  File_handle(const string& fn, const string& file_mode)
    : file_ptr{fopen(fn.c_str(), file_mode.c_str())}
  {
    if(file_ptr == nullptr){	// check if the file is properly opened
      throw runtime_error("Error. File cannot be opened properly.");
    }
  }
  ~File_handle(){fclose(file_ptr);}

  // the member functions are from the usual ways of using a file pointer.
  // Inspired from chapter 14 of another book "Practical C Programming"
  char fgetc(){return ::fgetc(file_ptr);} // read 1 character from ASCII (text) file
  // Aside: the :: qualifier without preceding namespace tells the compiler to look for the
  //        global namespace first. Without this ::, since this class also has the function
  //        named fgetc(), and compiler look from close namespace, it matches
  //        File_handle::fgetc() before matching global (original) fgetc(). To match the
  //        original one first, I put :: before fgetc().
  // https://stackoverflow.com/questions/4269034/
  void fputc(char ch){::fputc(ch, file_ptr);} // write 1 character to ASCII file

  // read size characters or until it hits '\n'. If ::fgets() hits the end of file, cstr stores
  // null
  void fgets(string& s, int size=1){	      
    char cstr[size+1];
    if(::fgets(cstr, size+1, file_ptr) == nullptr){
    // Those +1 is for the null character '\0' or 0. ::fgets(cstr, size, file_ptr) reads size-1
    // characters, and fill the last character with '\0'. So, to read size characters, we need
    // to put size+1 in fgets(). Then, it reads size characters, and add '\0' to the last, and
    // store it to cstr. Thus, cstr also needs size+1 characters.

      // ::fgets() returns nullptr if it hits the end of file
      s = "EOF";
      return;
    }
    s = cstr;		// it seems there is the appropriate assignment operator from
    // char* to string
  }

  // write the string to file
  void fputs(const string& s){
    if(::fputs(s.c_str(), file_ptr)){}
    // ::fputs() returns EOF (== -1 here) if error occured. Otherwise, it returns a non-negative
    // number.
    else
      throw runtime_error("Error in File_handle::fputs().");
  }

  // read binary file
  void fread(void *var_ptr, size_t read_bytes){
    // read "read_bytes" bytes from the current reading position
    char c[read_bytes];			// to check read error, I don't immediately store the read
    // bytes to var_ptr
    // If we use void* vp; instead of actually allocating some block of memory, when we try
    // to store read bytes to vp, segmentation fault happens, because there is nowhere to store
    // the read bytes.
    // Since bytes are not text characters, we don't have to care '\0' and add 1 to the
    // char array size
    
    size_t read_size{::fread(c, 1, read_bytes, file_ptr)};
    // check file error
    if(ferror(file_ptr))
      throw runtime_error("Error in File_handle::fread(). Reading failed.");

    // it's not always the case that the requested bytes are read. The byte size of the actually 
    // read bytes are stored in read_size.
    char *var_ptr_char{static_cast<char*>(var_ptr)}; // since void* cannot do subscripting
    // or arithmetic operation (e.g. it is invalid to do var_ptr[i] or var_ptr+i, since the
    // compiler cannot know the byte size of 1 element of the object pointed to by void*, thus
    // it cannot know how many bytes the program should forward the bytes with var_ptr+1),
    // I first convert var_ptr's type to char*. They (var_ptr_char and var_ptr) point to
    // the same address, but unlike void*, char* knows the byte size of 1 element of char, so
    // I can do subscripting or arithmetic operation on var_ptr_char now.
    for(int i=0; i<read_size; ++i)
      var_ptr_char[i] = c[i];
  }

  // write to binary file
  void fwrite(void *var_ptr, size_t write_bytes){
    ::fwrite(var_ptr, 1, write_bytes, file_ptr);
    if(ferror(file_ptr))
      throw runtime_error("Error in File_handle::fwrite(). Writing failed.");
  } 
private:
  FILE *file_ptr;
};


int main()
  try{

    File_handle fh("test.txt", "r");
    string l;
    fh.fgets(l, 100);
    cout << l;
    fh.fgets(l, 100);
    cout << l;
    fh.fgets(l, 100);
    cout << l;
    fh.fputs("test");

    File_handle fh3{"test.bin", "wb"}; // mode of writing binary file
    char var_cp1[5]{'t', 'e', 's', 't'}; // ['t', 'e', 's', 't', '\0']
    fh3.fwrite(var_cp1, sizeof(char)*4);  // write the 1st 4 letters, except '\0'
    // Although I wrote in binary format, the file size is the same as the text format (both
    // test.bin and test.txt have 4 bytes).


    File_handle fh2{"test.txt", "rb"}; // mode of reading binary file
    int var_i;
    fh2.fread(&var_i, sizeof(var_i));
    cout << hex << var_i << endl;


    // test if I can write a char value beyond ASCII code (128 or above) (unsigned char can have 
    // at most value 255) (char has sign bit)
    File_handle fh4{"test_other_than_ascii.bin", "wb"}; // mode of writing binary file
    unsigned char var_cp2[5]{255, 128, 't'};
    // unsigned char's 255 is the same in binary form as char's -1
    fh4.fwrite(var_cp2, sizeof(unsigned char)*3);  // write the 1st 4 letters, except '\0'

    // I was able to do this.
    // Can I do the same thing using text mode? Let's try
    // test if I can write characters beyond normal ASCII code (0~127) using normal write mode
    File_handle fh5{"test_other_than_ascii.txt", "w"}; // mode of writing text file
    unsigned char var_cp3[5]{255, 128, 't'};
    fh5.fputc(var_cp3[0]);
    fh5.fputc(var_cp3[1]);
    fh5.fputc(var_cp3[2]);
    // It worked the same way as the above binary write mode (using fh4)
    // Although fputc()'s argument type is char, and the passed var type is unsigned char, 
    // the compiler didn't complain about it, and the values are written as in the original
    // unsigned char array.
    // So I guess we don't need to use binary mode 'b'?
    // <- Basically, it seems true when reading each byte to a char. But some differences may 
    //    exist on some platform. For example, on Windows, in text writing mode, '\n' seems to 
    //    be automatically converted into '\r\n'.
    // https://stackoverflow.com/questions/229924/
    
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
