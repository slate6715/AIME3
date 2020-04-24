#ifndef MUD_H
#define MUD_H

#include <libconfig.h++>
#include "EntityDB.h"
#include "UserMgr.h"
#include "LogMgr.h"
#include "ActionMgr.h"

/***************************************************************************************
 * MUD - class that manages the mud as a whole. Each instance of a MUD class will be its
 *       own independent MUD.
 *
 ***************************************************************************************/
class MUD 
{
public:
   MUD();
   MUD(MUD &copy_from);
   virtual ~MUD();

	void initConfig();
	void loadConfig(const char *filename);

	void initialize();

	void bootServer(const char *ip_addr, unsigned short port);
	void startListeningThread();
	
	void runMUD();
	void cleanup();

	libconfig::Config *getConfig() { return &_mud_config; };

private:
   // Publicly-accessible attributes
	libconfig::Config _mud_config;

	LogMgr _mudlog;

	// Stores all the non-player entities in the game
	EntityDB _entity_db;

	ActionMgr _actions;

	// Stores and manages the players connected to the game
	UserMgr _users;

	bool _shutdown_mud = false;

	long _time_between_heartbeat;
};


#endif
