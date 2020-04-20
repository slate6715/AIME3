#ifndef MUD_H
#define MUD_H

#include <libconfig.h++>
#include "EntityDB.h"
#include "UserDB.h"
#include "LogMgr.h"

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


private:
   // Publicly-accessible attributes
	libconfig::Config _mud_config;

	// Stores all the non-player entities in the game
	EntityDB _entity_db;

	// Stores and manages the players connected to the game
	UserDB _user_db;

	LogMgr _mudlog;
};


#endif
