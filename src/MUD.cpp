#include <iostream>
#include "MUD.h"

namespace lc = libconfig;

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
		_entity_db(),
		_user_db(),
		_mudlog()
{


}


MUD::MUD(MUD &copy_from):
		_mud_config(),
		_entity_db(copy_from._entity_db),
		_user_db(copy_from._user_db),
		_mudlog(copy_from._mudlog)
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
	lc::Setting &root = _mud_config.getRoot();
	
	root.add("network", lc::Setting::TypeGroup);
	lc::Setting &network = root["network"];

	network.add("ip_addr", lc::Setting::TypeString) = "127.0.0.1";
	network.add("port", lc::Setting::TypeInt) = 6715;
	network.add("whitelist", lc::Setting::TypeBoolean) = false;	
	network.add("accesslist_file", lc::Setting::TypeString) = "blacklist.dat";

	root.add("misc", lc::Setting::TypeGroup);
	
	lc::Setting &misc = root["misc"];
	misc.add("logfile", lc::Setting::TypeString) = "mud.log";
	misc.add("loglvl", lc::Setting::TypeInt) = 1;

	root.add("infofiles", lc::Setting::TypeGroup);
	lc::Setting &info = root["info"];

	info.add("welcome", lc::Setting::TypeString) = "info/welcome.txt";
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
 * initialize - Set up the MUD for operation
 *
 *		Params:	logfile - opens/appends and starts logging at this location
 *					log_lvl - defines verbosity levels - 0 = no logging, 3 typically is max (though
 *                       there's no real limit)
 *
 *    Throws:  
 *
 *
 *********************************************************************************************/

void MUD::initialize() {

	lc::Setting &root = _mud_config.getRoot();
	lc::Setting &misc = root["misc"];

	// Start up the logs
	_mudlog.changeFilename(misc["logfile"]);
	_mudlog.setLogLvl(misc["log_lvl"]);

	
}

