#ifndef USERMGR_H
#define USERMGR_H

#include <map>
#include <memory>
#include <thread>
#include "TCPServer.h"
#include "Player.h"

/****************************************************************************************
 * UserMgr - class that stores and manages the connected players and provides methods for
 *				 loading, saving, and managing players and their connections. Also manages the
 *			    TCPServer listening socket for accepting new connections.
 *
 ****************************************************************************************/
class UserMgr 
{
public:
	UserMgr(LogMgr &mud_log);
   UserMgr(const UserMgr &copy_from);
   virtual ~UserMgr();

	// Initialize certain variables for this class from the config file
	void initialize(libconfig::Config &cfg_info);

	// Starts the incoming connection socket
	void startSocket(libconfig::Config &cfg_info);

	// Launches a thread that listens for new connections and user data 
	void startListeningThread(libconfig::Config &cfg_info);

	// Sends a signal to the listening thread to stop, then waits for the thread to
   // exit safely
	void stopListeningThread();

	// Checks for new users and, if authorized, adds them to the player list
	void checkNewUsers(const char *welcome_file);
	
	// Loop through all users, performing maintenance and executing their next
	// command via their handler
	void handleUsers();

	// Functions for loading and saving user info to disk 
	int loadUser(const char *username, Player &plr);
	
	bool saveUser(const char *username);
	bool saveUser(const Player &plr);

private:

	// The log manager passed in by reference on object creation
	LogMgr &_mud_log;

	// List of active users
	std::map<std::string, std::shared_ptr<Player>> _db;

	// Listening socket to accept new connections
	TCPServer _listen_sock;

	// A rolling index to assign to new user IDs until they fully login
	unsigned int _newuser_idx = 0;

	std::unique_ptr<std::thread> _listening_thread;
	bool _exit_listening_thread = false;

	std::string _infodir;
	std::string _userdir;
	std::string _welcomefile;
	std::string _motdfile;
};


#endif
