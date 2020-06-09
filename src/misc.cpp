#include <algorithm>
#include <termios.h>
#include <iostream>
#include <string>
#include <algorithm>
#include <random>
#include <functional>
#include <chrono>
#include <libconfig.h++>
#include <climits>
#include "misc.h"
#include "Player.h"
#include "global.h"

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
      tattr.c_lflag &= (tcflag_t) ~ECHO;
   else
      tattr.c_lflag |= (tcflag_t) ECHO;

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

/*******************************************************************************************
 * sendInfoFiles - sends a file or multiple files to the player, depending on if the config file
 *			has a single string or a list of strings
 *
 *		Params:	plr - where are we sending the files
 *					cfg - Used to get the infofiles directory
 *					log - record any issues accessing the files
 *				   ifile_setting - the text identifier for the config setting that defines the infofile
 *					
 *
 *******************************************************************************************/

void sendInfoFiles(std::shared_ptr<Player> plr, libconfig::Config &cfg, const char *ifile_setting) {

   // Set up the directory string
   std::string infodir;
   cfg.lookupValue("datadir.infodir", infodir);
   infodir += "/";

   libconfig::Setting &info_files = cfg.lookup(ifile_setting);

   std::string filename, path;
   int count = info_files.getLength();

   // If it's not a list, then just send it
   if (count == 0) {
      path = infodir;
      path += (const char *) info_files;
      plr->sendFile(path.c_str());
   } else {
      for (int i=0; i<count; i++) {
         path = infodir;
         path += (const char *) info_files[i];
         if (!plr->sendFile(path.c_str())) {
				std::string errmsg("Unable to open or read info file '");
				errmsg += (const char *) info_files[i];
				errmsg += "'";
				mudlog->writeLog(errmsg.c_str());
			}
      }
   }

}

unsigned int locateInTable(const char *name, const char **table) {
   std::string namestr = name;
   lower(namestr);

   size_t i=0;
   while ((table[i] != NULL) && (namestr.compare(table[i]) != 0))
      i++;

   if (table[i] == NULL)
      return UINT_MAX;
	return i;
}

/*******************************************************************************************
 * equalAbbrev - compares the size of buf against compare_str and returns true if identical.
 *					  allows for abbreviations when matching
 *
 *    Params:  buf - the string (of size n) compared against compare_str (any size)
 *             compare_str - 
 *
 *		Returns: true if equal up to size(buf) characters
 *
 *******************************************************************************************/

bool equalAbbrev(std::string &buf, const char *compare_str) {
	for (unsigned int i=0; i<buf.size(); i++) {
		if (*compare_str == '\0')
			return false;

		if (buf[i] != *compare_str)
			return false;

		compare_str++;
	}
	return true;
}

unsigned int getLineNumber(const char *filename, unsigned int offset) {
	unsigned int count = 0;
	FILE *fptr = fopen(filename, "r");
	if (fptr == NULL)
		return 0;

	for (unsigned int i=0; i<offset; i++) {
		int results = fgetc(fptr);
		if (results == EOF)
			return count;
		if (results == '\n')
			count++;
	}

	fclose(fptr);
	return count;
}


