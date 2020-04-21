#include <libconfig.h++>
#include <iostream>
#include "UserMgr.h"

namespace lc = libconfig;

/*********************************************************************************************
 * UserMgr (constructor) - 
 *
 *********************************************************************************************/
UserMgr::UserMgr(LogMgr &mud_log):
					_mud_log(mud_log),
					_db(),
					_listen_sock(_mud_log),
					_newuser_idx(0),
					_listening_thread(nullptr)
{


}


UserMgr::UserMgr(const UserMgr &copy_from):
					_mud_log(copy_from._mud_log),
					_db(copy_from._db),
					_listen_sock(copy_from._listen_sock),
					_newuser_idx(copy_from._newuser_idx),
					_listening_thread(nullptr)
{

}


UserMgr::~UserMgr() {

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

void UserMgr::startSocket(lc::Config &cfg_info) {
	
	std::string ip_addr;
	int port;


	// Will throw a SettingTypeException if these are not found
	cfg_info.lookupValue("network.ip_addr", ip_addr);
	cfg_info.lookupValue("network.port", port);

	if ((port < 0) || (port > 65535)) {
		std::string msg("Invalid port ");
		msg += port;
		msg += " in server config.";
		
		throw socket_error(msg.c_str());
	}

	// Bind the server - throws a socket_error if there's an issue
	_listen_sock.bindSvr(ip_addr.c_str(), (unsigned short) port);
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
	std::string welcome_file;
	cfg_info.lookupValue("infofiles.welcome", welcome_file);
	
	_exit_listening_thread = false;

	// ******* Lambda function for launching the thread ********
	_listening_thread = std::unique_ptr<std::thread>(new std::thread(
									[this, listening_loop, welcome_file](){

		long long interval = 1000000 / listening_loop;
		long long sleep_duration;

		while (!_exit_listening_thread) {
			auto start_loop = std::chrono::high_resolution_clock::now();

			// Check the listening socket for new connections
			checkNewUsers(welcome_file.c_str());
	
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
 *		Params:	welcome_file - the path/filename for the file to send to users
 *
 *********************************************************************************************/

void UserMgr::checkNewUsers(const char *welcome_file){

	TCPConn *new_conn = NULL;

	// While there's a new connection on the socket
	while ((new_conn = _listen_sock.handleSocket()) != NULL) {
		// Assign a rolling number for new users as userID
		std::string userid("newuser");
		userid += _newuser_idx++;
	
		// Create a new Player object with this connection and a temp userid
		std::shared_ptr<Player> new_plr(new Player(userid.c_str(), std::unique_ptr<TCPConn>{new_conn}, _mud_log));

		_db.insert(std::pair<std::string, std::shared_ptr<Player>>(userid, new_plr));

		new_plr->welcomeUser(welcome_file);

	}
}


/*********************************************************************************************
 * handleUsers - Loops through all users, performing maintenance and executing their next
 *					  command via their handler
 *
 *********************************************************************************************/

void UserMgr::handleUsers(){

	// Loop through the players
	auto plr_it = _db.begin();
	for (; plr_it != _db.end(); plr_it++) {
		Player &plr = (*plr_it->second);

		std::string cmd;
		if (plr.popCommand(cmd)) {
			std::cout << "Command: " << cmd << std::endl;
		}
	}
}


/*********************************************************************************************
 * loadUser - attempts to load the 
 *
 *********************************************************************************************/

int UserMgr::loadUser(const char *username) {
	return 1;	
}

