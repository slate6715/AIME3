#include <libconfig.h++>
#include "UserMgr.h"

namespace lc = libconfig;

/*********************************************************************************************
 * UserMgr:g (constructor) - 
 *
 *********************************************************************************************/
UserMgr:g::UserMgr(LogMgr &mud_log):
					_db(),
					_mud_log(mud_log)
{


}


UserMgr:g::UserMgr(const UserMgr &copy_from):
					_db(copy_from._db),
					_mud_log(copy_from._mud_log) 
{

}


UserMgr:g::~UserMgr() {

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
	unsigned short port;


	// Will throw a SettingTypeException if these are not found
	ip_addr = cfg_info["network.ip_addr"];
	port = std::dynamic_cast<unsigned short>(cfg_info["network.port"];

	// Bind the server - throws a socket_error if there's an issue
	_listen_socket.bindSvr(ip_addr, port);
	_listen_socket.listenSvr();
	
}

/*********************************************************************************************
 * checkNewUsers - Checks the socket for incoming connections and accepts them if they're
 *					    authorized on the access list
 *
 *********************************************************************************************/

void UserMgr::checkNewUsers(){

	TCPConn *new_conn = NULL;

	// While there's a new connection on the socket
	while ((new_conn = _listen_socket.handleSocket()) != NULL) {
		// Assign a rolling number for new users as userID
		std::string userid("newuser");
		userid.append(newuser_idx++);
	
		// Create a new Player object with this connection and a temp userid
		std::shared_ptr<Player> new_plr = new Player(userid.c_str(), std::unique_ptr<Player>(*new_conn));

		_db.push_back(new_plr);
		new_plr->welcomeUser();

	}
}




