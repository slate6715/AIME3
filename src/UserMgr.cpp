#include <libconfig.h++>
#include <iostream>
#include <boost/lexical_cast.hpp>
#include "UserMgr.h"
#include "EntityDB.h"
#include "MUD.h"
#include "misc.h"
#include "global.h"

namespace lc = libconfig;

/*********************************************************************************************
 * UserMgr (constructor) - 
 *
 *********************************************************************************************/
UserMgr::UserMgr():
					_db(),
					_listen_sock(),
					_newuser_idx(0),
					_listening_thread(nullptr),
					_infodir("data/info"),
					_userdir("data/users")
{


}


UserMgr::UserMgr(const UserMgr &copy_from):
					_db(copy_from._db),
					_listen_sock(copy_from._listen_sock),
					_newuser_idx(copy_from._newuser_idx),
					_listening_thread(nullptr),
					_infodir(copy_from._infodir),
					_userdir(copy_from._userdir)
{

}


UserMgr::~UserMgr() {

}

/*********************************************************************************************
 * initialize - pulls config settings required for this class from the config file into the object
 *
 *    Params:  cfg_info - The libconfig::Config object that stores all the necessary config info
 *                        to set up the user manager 
 *
 *********************************************************************************************/

void UserMgr::initialize(lc::Config &cfg_info) {
	cfg_info.lookupValue("datadir.infodir", _infodir);
	cfg_info.lookupValue("datadir.userdir", _userdir);

	std::string buf;	

	int timeval;
	cfg_info.lookupValue("network.conn_timeout", timeval);
	_conn_timeout = (time_t) timeval;

}

/*********************************************************************************************
 * startSocket - Creates the socket and starts it listening for new connections
 * 
 *		Params:	cfg_info - The libconfig::Config object that stores all the necessary config info
 *								  to set up the new connection
 *
 *		Throws:	libconfig::SettingTypeException - if a setting is missing, which signifies the
 *										config file was not set up correctly (a default is missing?)
					socket_error - there was an error binding or configuring the socket
 *
 *********************************************************************************************/

void UserMgr::startSocket(const char *ip_addr, unsigned short port) {
	
	// Bind the server - throws a socket_error if there's an issue
	_listen_sock.bindSvr(ip_addr, port);
	_listen_sock.listenSvr();
	
}

/*********************************************************************************************
 * startListeningThread - Launches a thread that loops through the listening socket and the
 *							     user connections, sending data in the output queue and receiving new
 *								  socket data, placing it in the input queue
 *
 *    Params:  cfg_info - The libconfig::Config object that stores all the necessary config
 *
 *    Throws:  libconfig::SettingTypeException - if a setting is missing, which signifies the
 *                            config file was not set up correctly (a default is missing?)
 *             socket_error - there was an error accessing a socket
 *					runtime_error - listening thread is already running
 *
 *********************************************************************************************/

void UserMgr::startListeningThread(lc::Config &cfg_info) {
	
	if (_listening_thread != nullptr) {
		throw std::runtime_error("UserMgr::startListeningthread - attempted to start a listening thread. One is already running");
	}

	// Get the number of times we should loop per second
	int listening_loop = 8;
	cfg_info.lookupValue("misc.listening_loop", listening_loop);
	if (listening_loop < 1) {
		mudlog->writeLog("ERROR - Config setting listening_loop is less than 1 and invalid. Defaulting to 8.\n");
		listening_loop = 8;
	}

	_exit_listening_thread = false;

	// ******* Lambda function for launching the thread ********
	_listening_thread = std::unique_ptr<std::thread>(new std::thread(
									[this, listening_loop, &cfg_info](){

		long long interval = 1000000 / listening_loop;
		long long sleep_duration;

		while (!_exit_listening_thread) {
			auto start_loop = std::chrono::high_resolution_clock::now();

			// Check the listening socket for new connections
			checkNewUsers(cfg_info);
	
			// Loop through our players, handling their connection data	
			auto user_it = _db.begin();
			for (; user_it != _db.end(); user_it++) {
				user_it->second->handleConnection(_conn_timeout);		
			}
			
			// Sleep the appropriate interval
			auto elapsed = std::chrono::high_resolution_clock::now() - start_loop;
			sleep_duration = interval - std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
			usleep(sleep_duration);
		}
	})); // End lambda function for thread
}

/*********************************************************************************************
 * stopListeningThread - signals for the thread to exit by marking a shared boolean as true,
 *							    then joins the thread to wait for it to completely exit
 *
 *
 *********************************************************************************************/
void UserMgr::stopListeningThread() {
	_exit_listening_thread = true;

	// Will block until the thread exits
	_listening_thread->join();
	_listening_thread.reset();
}


/*********************************************************************************************
 * checkNewUsers - Checks the socket for incoming connections and accepts them if they're
 *					    authorized on the access list
 *
 *		Params:	mud_cfg - the config with all the files to send to users when they connect
 *
 *********************************************************************************************/

void UserMgr::checkNewUsers(libconfig::Config &mud_cfg){

	TCPConn *new_conn = NULL;

	// While there's a new connection on the socket
	while ((new_conn = _listen_sock.handleSocket()) != NULL) {
		// Assign a rolling number for new users as userID
		std::string userid("player:newuser" + boost::lexical_cast<std::string>(_newuser_idx++));
	
		// Create a new Player object with this connection and a temp userid
		std::shared_ptr<Player> new_plr(new Player(userid.c_str(), std::unique_ptr<TCPConn>{new_conn}));
		new_plr->setSelfPtr(new_plr);

		_db.insert(std::pair<std::string, std::shared_ptr<Player>>(userid, new_plr));

		new_plr->welcomeUser(mud_cfg, new_plr);

	}
}


/*********************************************************************************************
 * handleUsers - Loops through all users, performing maintenance and executing their next
 *					  command via their handler
 *
 *		Params:	actions - used to lookup Actions and place new ones in the queue based on user
 *							    input.
 *
 *********************************************************************************************/

void UserMgr::handleUsers(libconfig::Config &cfg_info, EntityDB &edb){

	// Loop through the players
	auto plr_it = _db.begin();
	while (plr_it != _db.end()) {
		Player &plr = (*plr_it->second);

		// If the connection is closed, remove the player
		if (plr_it->second->getConnStatus() == TCPConn::Closed) {
			plr_it = _db.erase(plr_it);
		}

		// Update the player prompts
		plr.updatePrompt();

		std::string cmd;
		if (plr.popCommand(cmd)) {
			int results;

			// If the handler returns other than 0, then we need to do something
			if ((results = plr.handleCommand(cmd)) > 0) {

				// The handler is ready to be popped
				if (results == 1) {
					std::string haction;
					std::vector<std::string> hresults;
					plr.popHandler(hresults);

					// This was a LoginHandler and the user just successfully logged in
					if ((hresults.size() > 0) && (hresults[0] == "loggedin")) {
						// We need to remove the player from the user list, change their name, and re-add
						std::string userkey("player:");
						userkey += hresults[1];

						// Make sure this doesn't get destroyed
						std::shared_ptr<Player> pptr = plr_it->second;

						// Erase this player from the user list
						plr_it = _db.erase(plr_it);
		
						// Now re-add the player with their actual name
						plr.setID(userkey.c_str());
						auto newplr_it = _db.insert(std::pair<std::string, std::shared_ptr<Player>>(userkey, pptr));

						std::string startloc;
						cfg_info.lookupValue("gameplay.startloc", startloc);

						std::shared_ptr<Physical> curloc;
						if ((curloc = edb.getPhysical(startloc.c_str())) == nullptr) {
							std::string msg("Unable to assign incoming player to start loc '");
							msg += startloc;
							msg += "' defined in config file.";
							mudlog->writeLog(msg.c_str());
							
							// Add code to boot the player
							plr.sendMsg("Unable to assign you to a start location. Login failed.\n");
							continue;	
						}
			
						plr.movePhysical(curloc, newplr_it.first->second);


						plr.sendCurLocation();
//						plr.sendPrompt();
						continue;	
					}
				}
				else {
					std::string msg("Message hander returned unexpected results for player: ");
					msg += plr.getID();
					mudlog->writeLog(msg.c_str());
				}

			}

			
		}
		plr_it++;
	}
}


/*********************************************************************************************
 * loadUser - attempts to load the user into the given Player object
 *				  
 *		Params:	username - self-explanatory
 *					plr - an existing Player class that will be populated (note: id will not be
 *						   changed and should not be until authentication happens)
 *
 *		Returns:	1 if loaded, 0 if not found
 *
 *********************************************************************************************/

int UserMgr::loadUser(const char *username, Player &plr) {
	return plr.loadUser(_userdir.c_str(), username);
}

/*********************************************************************************************
 * saveUser - saves the user data to a file
 *
 *    Params:  username - self-explanatory
 *
 *    Returns: 1 if loaded, 0 if not found
 *
 *********************************************************************************************/

bool UserMgr::saveUser(const char *username) {
	// Find the user
	auto plrit = _db.find(username);
	if (plrit == _db.end())
		return false;

	Player &plr = *(plrit->second);
	return saveUser(plr);
}

bool UserMgr::saveUser(const Player &plr) {
	return plr.saveUser(_userdir.c_str());
}

/*********************************************************************************************
 * getPlayer - gets the player based off the parameter
 *
 *    Params:  name - the name to search for
 *             allow_abbrev - if true, search will return the first match to an abbreviated name
 *
 *    Returns: pointer to the player if found, nullptr if not
 *
 *********************************************************************************************/

std::shared_ptr<Player> UserMgr::getPlayer(const char *name, bool allow_abbrev) {
   std::string namestr = name;

   auto plr_it = _db.begin();
   for ( ; plr_it != _db.end(); plr_it++) {
      if ((allow_abbrev) && (namestr.compare(0, namestr.size(), plr_it->second->getID()) == 0))
         return plr_it->second;
      else if (!allow_abbrev && (namestr.compare(plr_it->second->getID()) == 0))
         return plr_it->second;
   }
   return nullptr;
}

/*********************************************************************************************
 * showUsers - displays the list of logged on users
 *
 *********************************************************************************************/

const char *UserMgr::showUsers(std::string &buf) {

	int count = 0;
	std::string name;
	std::stringstream str;
	buf.clear();

	str << "Users: \n-----------------------\n";

	auto plr_it = _db.begin();
	for ( ; plr_it != _db.end(); plr_it++) {
		  plr_it->second->getNameID(name);
		
		name[0] = toupper(name[0]);
		str << name << "\n";
		count++;
	}
	str << "-----------------------\n" << count;
	if (count == 1)
		str << " user logged on\n\n"; 
	else
		str << " users logged on\n\n";
	buf = str.str();
	return buf.c_str();	
}

/*********************************************************************************************
 * sendMsg - sends a message to the users who meet certain criteria
 *
 *********************************************************************************************/

int UserMgr::sendMsg(const char *msg, std::vector<std::string> *exclude_flags,
                                std::vector<std::string> *required_flags,
                                std::shared_ptr<Physical> exclude_ind) {

	int count = 0;

	// Loop through all connected users
	auto p_it = _db.begin();
	for ( ; p_it != _db.end(); p_it++) {
		// Exclude the person if they're exclude_ind
		if (exclude_ind == (*p_it).second)
			continue;

		// Check exclude flags
		if (exclude_flags != NULL) {
			// Loop through our flags, checking the player's flags
			for (unsigned int i=0; i<exclude_flags->size(); i++) {
				if (p_it->second->isFlagSet((*exclude_flags)[i].c_str()))
					continue;	// skip this player
			}
		}

		// Check required flags
		if (required_flags != NULL) {
         // Loop through our flags, checking the player's flags
         for (unsigned int i=0; i<required_flags->size(); i++) {
            if (!p_it->second->isFlagSet((*required_flags)[i].c_str()))
               continue;   // skip this player

			}
		}
		p_it->second->sendMsg(msg);
		count++;
	}
	return count;
}


