#include <libconfig.h++>
#include <iostream>
#include <chrono>
#include "ActionMgr.h"
#include "misc.h"

namespace lc = libconfig;

/*********************************************************************************************
 * ActionMgr (constructor) - 
 *
 *********************************************************************************************/
ActionMgr::ActionMgr(LogMgr &mud_log):
					_mud_log(mud_log),
					_action_db(),
					_action_queue()
{


}


ActionMgr::ActionMgr(const ActionMgr &copy_from):
					_mud_log(copy_from._mud_log),
					_action_db(copy_from._action_db),
					_action_queue(copy_from._action_queue)
{

}


ActionMgr::~ActionMgr() {

}

/*********************************************************************************************
 * initialize - pulls config settings required for this class from the config file into the object
 *
 *    Params:  cfg_info - The libconfig::Config object that stores all the necessary config info
 *                        to set up the user manager 
 *
 *********************************************************************************************/

void ActionMgr::initialize(lc::Config &cfg_info) {
/*	cfg_info.lookupValue("datadir.infodir", _infodir);
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
*/
}

/*********************************************************************************************
 * handleActions - goes through the action queue, executing those actions whose timer is < now().
 *				This function basically handles the dyanmics of the game. All entities that are
 *				"doing something" are doing it in this function using an action in the queue
 *
 *    Params:  cfg_info - The libconfig::Config object that stores all the necessary config info
 *                        to set up the user manager
 *
 *********************************************************************************************/

void ActionMgr::handleActions() {


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

/*void ActionMgr::startSocket(lc::Config &cfg_info) {
	
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
	
}*/

