#include <iostream>
#include "MUD.h"
#include "global.h"

namespace lc = libconfig;

LogMgr *mudlog = NULL;

/*********************************************************************************************
 * MUD (constructor) - creates our ReplServer. Initializes:
 *
 *    verbosity - passes this value into QueueMgr and local, plus each connection
 *    _time_mult - how fast to run the simulation - 2.0 = 2x faster
 *    ip_addr - which ip address to bind the server to
 *    port - bind the server here
 *
 *********************************************************************************************/
MUD::MUD():
		_mud_config(),
		_mudlog(),
		_entity_db(),
		_actions(),
		_users(*this),
		_time_between_heartbeat(100000)
{


}


MUD::MUD(MUD &copy_from):
		_mud_config(),
		_mudlog(copy_from._mudlog),
		_entity_db(copy_from._entity_db),
		_actions(copy_from._actions),
		_users(copy_from._users),
		_time_between_heartbeat(copy_from._time_between_heartbeat)
{

}


MUD::~MUD() {

}

/*********************************************************************************************
 * initConfig - initializes the Config to default settings. Typically called before reading in
 *				    the config file
 *
 *
 *********************************************************************************************/

void MUD::initConfig() {
/*	lc::Setting &root = _mud_config.getRoot();
	
	root.add("network", lc::Setting::TypeGroup);
	lc::Setting &network = root["network"];

	network.add("ip_addr", lc::Setting::TypeString) = "127.0.0.1";
	network.add("port", lc::Setting::TypeInt) = 6715;
	network.add("whitelist", lc::Setting::TypeBoolean) = false;	
	network.add("accesslist_file", lc::Setting::TypeString) = "blacklist.dat";

	root.add("misc", lc::Setting::TypeGroup);
	
	lc::Setting &misc = root["misc"];
	misc.add("logfile", lc::Setting::TypeString) = "mud.log";

	std::string logfile;
	_mud_config.lookupValue("misc.logfile", logfile);
	std::cout << "Logfile: " << logfile << std::endl;

	misc.add("loglvl", lc::Setting::TypeInt) = 1;

	root.add("infofiles", lc::Setting::TypeGroup);
	lc::Setting &info = root["infofiles"];

	info.add("welcome", lc::Setting::TypeString) = "info/welcome.txt";*/
}

/*********************************************************************************************
 * loadConfig - loads in the config file specified in the parameter
 *
 *		Throws:	FileIOException - there was an issue reading the file
 *					ParseException - error parsing the file (getError, getFile, getLine available)
 *
 *
 *********************************************************************************************/

void MUD::loadConfig(const char *filename) {
	_mud_config.readFile(filename);

}

/*********************************************************************************************
 * startLog - configures the log and assigns the global pointer
 *
 *    Throws:
 *
 *
 *********************************************************************************************/
void MUD::startLog() {
	std::string cstr_setting;
	int cint_setting;
	
   _mud_config.lookupValue("misc.logfile", cstr_setting);
   _mud_config.lookupValue("misc.loglvl", cint_setting);

   _mudlog.changeFilename(cstr_setting.c_str());
   _mudlog.setLogLvl((unsigned int) cint_setting);
   mudlog = &_mudlog;
}

/*********************************************************************************************
 * initialize - Set up the MUD for operation
 *
 *    Throws:  
 *
 *
 *********************************************************************************************/

void MUD::initialize() {

	// Start up the logs
	int heartbeat_per_sec;

	_mud_config.lookupValue("misc.heartbeat_per_sec", heartbeat_per_sec);
	_time_between_heartbeat = 1000000 / heartbeat_per_sec;

	// Init the user database
	_users.initialize(_mud_config);
	
	// Init out actions manager
	_actions.initialize(_mud_config);

	// Load al traits
	_entity_db.loadTraits(_mud_config);

	// Load all entities
	_entity_db.loadEntities(_mud_config);
}

/*********************************************************************************************
 * bootServer - Sets up the listening socket and 
 *
 *
 *    Throws: socket_error - issues binding or otherwise bringing the server online
 *
 *
 *********************************************************************************************/

void MUD::bootServer(const char *ip_addr, unsigned short port) {
	_users.startSocket(ip_addr, port);
}

/*********************************************************************************************
 * startListeningThread - Launches a thread in UserMgr that listens for new connections and
 *								  also sends/receives network traffic from user connections
 *
 *
 *    Throws: socket_error - issues binding or otherwise bringing the server online
 *
 *
 *********************************************************************************************/

void MUD::startListeningThread() {
	_users.startListeningThread(_mud_config);
}
	

/*********************************************************************************************
 * runMUD - starts the MUD's main loop and does not exit until a command to do so is given
 *
 *    Throws: 
 *
 *
 *********************************************************************************************/

void MUD::runMUD() {

	// Main mud loop
	while (!_shutdown_mud) {
		
		std::chrono::system_clock::time_point start = std::chrono::system_clock::now();

		// Goes through the user's handlers, creating actions as required on the queue 
		_users.handleUsers(_mud_config, _entity_db);

		// Go through the actions in the queue, handling those that are being executed now
		_actions.handleActions(*this);

		std::chrono::system_clock::time_point end = std::chrono::system_clock::now();

		long sleep_duration = _time_between_heartbeat - 
										std::chrono::duration_cast<std::chrono::microseconds>(end-start).count();
		if (sleep_duration > 0)
			usleep(sleep_duration);
	}

}

/*********************************************************************************************
 * cleanup - cleans up things in the MUD and preps for shutdown
 *
 *    Throws:
 *
 *
 *********************************************************************************************/

void MUD::cleanup() {
	_users.stopListeningThread();

}

