#include <string>
#include <libconfig.h++>
#include <boost/filesystem.hpp>

class Player;
class LogMgr;

// Remove /r and /n from a string
void clrNewlines(std::string &str);

// Removes spaces from leading and trailing edge
void clrSpaces(std::string &str);

// Takes the orig string and splits it into left and right sides around a delimiter
bool split(std::string &orig, std::string &left, std::string &right, const char delimiter);

// Turns a string into lowercase
void lower(std::string &str);

// Turns off local echo from a user's terminal
int hideInput(int fd, bool hide);

// Generates a random string of the assigned length
void genRandString(std::string &buf, size_t n);

// sends a file or multiple files to the player, depending on if the config file
// has a single string or a list of strings
void sendInfoFiles(std::shared_ptr<Player> plr, libconfig::Config &cfg, const char *ifile_setting);

// Used for iterating files in a directory
struct path_leaf_string 
{
	std::string operator()(const boost::filesystem::directory_entry &entry) const
	{
		return entry.path().leaf().string();
	}
};
