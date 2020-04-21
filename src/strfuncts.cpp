#include <algorithm>
#include <termios.h>
#include <iostream>
#include <string>
#include <algorithm>
#include <random>
#include <functional>
#include <chrono>
#include "strfuncts.h"

/*******************************************************************************************
 * clrNewlines - removes \r and \n from the string passed into buf
 *******************************************************************************************/
void clrNewlines(std::string &str) {
   str.erase(std::remove(str.begin(), str.end(), '\r'), str.end());
   str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());
}

/*******************************************************************************************
 * clrSpaces - removes spaces from the left and right side of the string
 *******************************************************************************************/
void clrSpaces(std::string &str) {
   const auto begin = str.find_first_not_of(" ");
   
   const auto end = str.find_last_not_of(" ");
   str = str.substr(begin, end - begin + 1);
}

/*******************************************************************************************
 * split - takes the orig string and places substrings into left and right, divided by the
 *         first delimiter found
 *******************************************************************************************/

bool split(std::string &orig, std::string &left, std::string &right, const char delimiter) {
   std::string::size_type del_loc = orig.find(delimiter);
   if (del_loc == std::string::npos)
      return false;

   left = orig.substr(0, del_loc);
   right = orig.substr(del_loc + 1, orig.size());
   right.erase(std::remove(right.begin(), right.end(), '\n'), right.end());
   right.erase(std::remove(right.begin(), right.end(), '\r'), right.end());
   lower(left);

   return true;
}

/*******************************************************************************************
 * lower - simply converts the passed in string to lowercase
 *******************************************************************************************/

void lower(std::string &str) {
   std::transform(str.begin(), str.end(), str.begin(),
         [](unsigned char c){ return std::tolower(c); });
}

/*******************************************************************************************
 * hideInput - turns on/off the fd's local echo (normally fd=stdin)
 *
 *    Params:  fd - the fd to turn off local echo
 *             hide - true = echo off, false = echo on (show)
 *******************************************************************************************/

int hideInput(int fd, bool hide) {
   struct termios tattr;

   if (tcgetattr(fd, &tattr) != 0)
      return -1;

   if (hide)
      tattr.c_lflag &= ~ECHO;
   else
      tattr.c_lflag |= ECHO;

   if (tcsetattr(fd, TCSAFLUSH, &tattr) != 0)
      return -1; 
   return 0;
}

/*******************************************************************************************
 * genRandString - generates a random string of length n and places in buf
 *
 *******************************************************************************************/

void genRandString(std::string &buf, size_t n) {
   unsigned int seed = std::chrono::system_clock::now().time_since_epoch().count();   
   std::default_random_engine rndgen(seed);
   std::uniform_int_distribution<unsigned short> dist(32, 126);
   auto char_gen = std::bind(dist, rndgen);
  
   buf.clear(); 
   buf.reserve(n);
   for (size_t i=0; i<n; i++)
      buf += char_gen();
}


