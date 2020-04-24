#include <libconfig.h++>
#include <iostream>
#include <boost/lexical_cast.hpp>
#include "UserMgr.h"
#include "misc.h"

namespace lc = libconfig;

/*********************************************************************************************
 * UserMgr (constructor) - 
 *
 *********************************************************************************************/
UserMgr::UserMgr(LogMgr &mud_log, ActionMgr &actions):
					_mud_log(mud_log),
					_actions(actions),
					_db(),
					_listen_sock(_mud_log),
					_newuser_idx(0),
					_listening_thread(nullptr),
					_infodir("data/info"),
					_userdir("data/users"),
					_welcomefile("welcome.info")
{


}


UserMgr::UserMgr(const UserMgr &copy_from):
					_mud_log(copy_from._mud_log),
					_actions(copy_from._actions),
					_db(copy_from._db),
					_listen_sock(copy_from._listen_sock),
					_newuser_idx(copy_from._newuser_idx),
					_listening_thread(nullptr),
					_infodir(copy_from._infodir),
					_userdir(copy_from._userdir),
					_welcomefile(copy_from._welcomefile)
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
	cfg_info.lookupValue("infofiles.welcome", buf);
	_welcomefile = _infodir;
	_welcomefile += "/";
	_welcomefile += buf;

	cfg_info.lookupValue("infofiles.motd", buf);
	_motdfile = _infodir;
	_motdfile += "/";
	_motdfile += buf;

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
		_mud_log.writeLog("ERROR - Config setting listening_loop is less than 1 and invalid. Defaulting to 8.\n");
		listening_loop = 8;
	}

	// Grab the welcome file location
	std::string welcome_file(_infodir), buf;
	welcome_file += "/";
	cfg_info.lookupValue("infofiles.welcome", buf);
	welcome_file += buf;	

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
				user_it->second->handleConnection();		
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
		std::string userid("player@newuser" + boost::lexical_cast<std::string>(_newuser_idx++));
	
		// Create a new Player object with this connection and a temp userid
		std::shared_ptr<Player> new_plr(new Player(userid.c_str(), std::unique_ptr<TCPConn>{new_conn}, _mud_log));

		_db.insert(std::pair<std::string, std::shared_ptr<Player>>(userid, new_plr));

		new_plr->welcomeUser(mud_cfg, _actions, new_plr);

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

void UserMgr::handleUsers(){

	// Loop through the players
	auto plr_it = _db.begin();
	while (plr_it != _db.end()) {
		Player &plr = (*plr_it->second);

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
					if (hresults[0] == "loggedin") {
						// We need to remove the player from the user list, change their name, and re-add
						std::string userkey("player@");
						userkey += hresults[1];

						// Make sure this doesn't get destroyed
						std::shared_ptr<Player> pptr = plr_it->second;

						// Erase this player from the user list
						plr_it = _db.erase(plr_it);
		
						// Now re-add the player with their actual name
						plr.setID(userkey.c_str());
						_db.insert(std::pair<std::string, std::shared_ptr<Player>>(userkey, pptr));

						// Send the MOTD to the user
						plr.sendFile(_motdfile.c_str());
						plr.sendPrompt();
						continue;	
					}
				}
				else {
					std::string msg("Message hander returned unexpected results for player: ");
					msg += plr.getID();
					_mud_log.writeLog(msg.c_str());
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

