#ifndef USERMGR_H
#define USERMGR_H

#include <map>
#include <memory>
#include <thread>
#include <libconfig.h++>
#include "TCPServer.h"
#include "Player.h"
#include "ActionMgr.h"
#include "EntityDB.h"

/****************************************************************************************
 * UserMgr - class that stores and manages the connected players and provides methods for
 *				 loading, saving, and managing players and their connections. Also manages the
 *			    TCPServer listening socket for accepting new connections.
 *
 ****************************************************************************************/
class UserMgr 
{
public:
	UserMgr(MUD &engine);
   UserMgr(const UserMgr &copy_from);
   virtual ~UserMgr();

	// Initialize certain variables for this class from the config file
	void initialize(libconfig::Config &cfg_info);

	// Starts the incoming connection socket
	void startSocket(const char *ip_addr, unsigned short port);

	// Launches a thread that listens for new connections and user data 
	void startListeningThread(libconfig::Config &cfg_info);

	// Sends a signal to the listening thread to stop, then waits for the thread to
   // exit safely
	void stopListeningThread();

	// Checks for new users and, if authorized, adds them to the player list
	void checkNewUsers(libconfig::Config &mud_cfg);

   // Removes all references to this item from the player objects
   size_t purgePhysical(std::shared_ptr<Physical> item);

	// Remove a player from the database - all shared pointer refs must be purged
	void removePlayer(Player *plr);

	// Displays the list of logged on users, populating the buffer
	const char *showUsers(std::string &buf);

	// Loop through all users, performing maintenance and executing their next
	// command via their handler
	void handleUsers(libconfig::Config &cfg_info, EntityDB &edb);

	// Functions for loading and saving user info to disk 
	int loadUser(const char *username, Player &plr);
	
	bool saveUser(const char *username);
	bool saveUser(const Player &plr);

	std::shared_ptr<Player> getPlayer(const char *name, bool allow_abbrev = true);

	int sendMsg(const char *msg, std::vector<std::string> *exclude_flags,
										  std::vector<std::string> *require_flags, 
										  std::shared_ptr<Physical> exclude_ind);

private:

	// Used by the handlers to queue up commands/actions to be executed
	MUD &_engine;

	// List of active users
	std::map<std::string, std::shared_ptr<Player>> _db;

	// Listening socket to accept new connections
	TCPServer _listen_sock;

	// A rolling index to assign to new user IDs until they fully login
	unsigned int _newuser_idx = 0;

	time_t _conn_timeout = 30;

	std::unique_ptr<std::thread> _listening_thread;
	bool _exit_listening_thread = false;

	std::string _infodir;
	std::string _userdir;
};


#endif
