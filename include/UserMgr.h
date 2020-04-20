#ifndef USERMGR_H
#define USERMGR_H

#include <map>
#include <memory>
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

	// Starts the incoming connection socket
	void startSocket(libconfig::Config &cfg_info);

	// Checks for new users and, if authorized, adds them to the player list
	void checkNewUsers();
		
 
private:

	// The log manager passed in by reference on object creation
	LogMgr &_mud_log;

	// List of active users
	std::map<std::string, std::shared_ptr<Player>> _db;

	// Listening socket to accept new connections
	TCPServer _listen_sock;

	// A rolling index to assign to new user IDs until they fully login
	unsigned int newuser_idx = 0;
};


#endif
