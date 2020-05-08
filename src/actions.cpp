#include <iostream>
#include <libconfig.h++>
#include <regex>
#include <memory>
#include "actions.h"
#include "MUD.h"
#include "Action.h"
#include "misc.h"
#include "Location.h"
#include "global.h"
#include "Getable.h"


/*******************************************************************************************
 * infocom - displays an info file to the user 
 *******************************************************************************************/
int infocom(MUD &engine, Action &act_used) {

	std::shared_ptr<Organism> agent = act_used.getAgent();


	libconfig::Config &mud_cfg = *(engine.getConfig());

	// First, check to make sure the request uses only letters and/or numbers
	std::regex fncheck("[a-z0-9]+");
	std::string infoname = act_used.getToken(1);
	lower(infoname);
	
	if (!std::regex_match(infoname, fncheck)) {
		agent->sendMsg("The info filename you are seeking can only consist of letters and numbers.\n");
		return 0;		
	}

	std::string fullpath;
	mud_cfg.lookupValue("datadir.infodir", fullpath);

	fullpath += "/";
	fullpath += act_used.getToken(1);
	fullpath += ".info";

	if (!agent->sendFile(fullpath.c_str())) {
		agent->sendMsg("That info file does not appear to exist.\n");
		return 0;
	}
	
	
	return 1;
}

/*******************************************************************************************
 * gocom - moves from room to room 
 *******************************************************************************************/
int gocom(MUD &engine, Action &act_used) {
	std::string dir;
	(void) engine;

	if (act_used.numTokens() >= 2)
		dir = act_used.getToken(1);
	else
		dir = act_used.getToken(0);

	std::shared_ptr<Organism> agent = act_used.getAgent();
	std::shared_ptr<Location> cur_loc = std::dynamic_pointer_cast<Location>(agent->getCurLoc());

	if (cur_loc == nullptr) {
		std::stringstream msg;
		msg << "SERIOUS ERROR: gocom function, agent " << agent->getID() << 
					" current location not a Location class or not set.";
		mudlog->writeLog(msg.str().c_str());

		agent->sendMsg("SERIOUS ERROR: You do not have a valid current location. See an Admin.\n");
		return 0;
	}

	std::shared_ptr<Location> exit_loc = cur_loc->getExitAbbrev(dir.c_str());

	if (exit_loc == nullptr) {
		agent->sendMsg("There is no exit that direction.\n");
		return 0;
	}

	agent->moveEntity(exit_loc);
	agent->sendCurLocation();

	return 1;
}

/*******************************************************************************************
 * lookcom - used to display a room or to examine something
 *******************************************************************************************/
int lookcom(MUD &engine, Action &act_used) {
	std::shared_ptr<Organism> agent = act_used.getAgent();
	(void) engine; // Eliminate compile warnings

	// If they just typed look or examine
	if (act_used.numTokens() == 1) {	
		agent->sendCurLocation();
		return 1;
	}

	// They seem to want to look in something?
	std::string preposition = act_used.getToken(1);
	lower(preposition);

	// If they didn't type a preposition, we assume "at"
	if ((preposition.compare("at")) != 0 && (preposition.compare("in") != 0))
		preposition = "at";

	// Examine something
	if (preposition.compare("at") == 0) {
		agent->sendMsg(act_used.getTarget1()->getDesc());
		return 1;
	}

	// Look inside something (TBD)

	return 1;
}

/*******************************************************************************************
 * exitscom - used to show all the exits from this location
 *******************************************************************************************/
int exitscom(MUD &engine, Action &act_used) {
   std::shared_ptr<Organism> agent = act_used.getAgent();
   (void) engine; // Eliminate compile warnings

	agent->sendMsg("\n");
   agent->sendExits();
	agent->sendMsg("\n");
   return 1;
}

/*******************************************************************************************
 * drocom - drop something from the player's inventory
 *******************************************************************************************/

int dropcom(MUD &engine, Action &act_used) {
   std::shared_ptr<Organism> agent = act_used.getAgent();
   (void) engine; // Eliminate compile warnings

	std::shared_ptr<Getable> gptr = std::dynamic_pointer_cast<Getable>(act_used.getTarget1());

	if (gptr == nullptr) {
		agent->sendMsg("You don't seem to have that.\n");
		return 0;
	} else if (gptr->isFlagSet("NoDrop")) {
		agent->sendMsg("You are unable to drop the ");
		agent->sendMsg(gptr->getTitle());
		agent->sendMsg("\n");
		return 0;
	}

	gptr->moveEntity(agent->getCurLoc(), act_used.getTarget1());
	std::string msg("You drop the ");
	msg += gptr->getTitle();
	msg += "\n";
	agent->sendMsg(msg.c_str());

   return 1;
}

/*******************************************************************************************
 * getcom - get something from the ground or a container
 *******************************************************************************************/
int getcom(MUD &engine, Action &act_used) {
   std::shared_ptr<Organism> agent = act_used.getAgent();
   (void) engine; // Eliminate compile warnings

   std::shared_ptr<Getable> gptr = std::dynamic_pointer_cast<Getable>(act_used.getTarget1());
   std::shared_ptr<Entity> cur_loc = agent->getCurLoc();

   gptr->moveEntity(agent, act_used.getTarget1());
   std::string msg("You pick up the ");
   msg += gptr->getTitle();
   msg += "\n";
   agent->sendMsg(msg.c_str());

   gptr->popRoomDesc();

   return 1;
}

/*******************************************************************************************
 * inventorycom - display the player's inventory
 *******************************************************************************************/
int inventorycom(MUD &engine, Action &act_used) {
   std::shared_ptr<Organism> agent = act_used.getAgent();
   (void) engine; // Eliminate compile warnings

	agent->sendMsg("Inventory contains:\n");

	std::string buf;

	agent->sendMsg(agent->listContents(buf));
	agent->sendMsg("\n");
	return 1;
}

/*******************************************************************************************
 * userscom - shows the users that are currently logged on
 *******************************************************************************************/
int userscom(MUD &engine, Action &act_used) {
	UserMgr &umgr = *engine.getUserMgr();

   std::shared_ptr<Organism> agent = act_used.getAgent();

	std::string buf;
	agent->sendMsg(umgr.showUsers(buf));	
	return 1;
}

/*******************************************************************************************
 * saycom - speak to people in the room
 *******************************************************************************************/
int saycom(MUD &engine, Action &act_used) {
	(void) engine;
   std::shared_ptr<Organism> agent = act_used.getAgent();

	std::stringstream msg;
	std::string name;
	agent->getNameID(name);
	name[0] = toupper(name[0]);
	
	msg << name << " says '" << act_used.getToken(1) << "'\n";
	agent->getCurLoc()->sendMsg(msg.str().c_str(), agent);

	msg.str("");
	msg << "You say '" << act_used.getToken(1) << "'\n";
	agent->sendMsg(msg.str().c_str());
	return 1;
}

/*******************************************************************************************
 * chatcom - speak to people in the mud who have joined the chat channel
 *******************************************************************************************/
int chatcom(MUD &engine, Action &act_used) {
   std::shared_ptr<Organism> agent = act_used.getAgent();
	UserMgr &umgr = *engine.getUserMgr();

   std::stringstream msg;
   std::string name;
   agent->getNameID(name);
   name[0] = toupper(name[0]);

	std::vector<std::string> ex_flags;
	ex_flags.push_back("nochat");

   msg << name << " chats '" << act_used.getToken(1) << "'\n";

	// Send this to everyone who does not have nochat set and is not the agent
	umgr.sendMsg(msg.str().c_str(), &ex_flags, NULL, agent);

   msg.str("");
   msg << "You chat '" << act_used.getToken(1) << "'\n";
   agent->sendMsg(msg.str().c_str());

	return 1;
}

/*******************************************************************************************
 * tellcom - tell something to a specific player
 *******************************************************************************************/
int tellcom(MUD &engine, Action &act_used) {
	(void) engine;

   std::shared_ptr<Organism> agent = act_used.getAgent();
	std::string pname, tname;
	std::stringstream msg;

	agent->getNameID(pname);
	act_used.getTarget1()->getNameID(tname);
	pname[0] = toupper(pname[0]);
	tname[0] = toupper(tname[0]);
   msg << pname << " tells you '" << act_used.getToken(2) << "'\n";

   // Send this to everyone who does not have nochat set and is not the agent
   act_used.getTarget1()->sendMsg(msg.str().c_str());

   msg.str("");
   msg << "You tell " << tname << " '" << act_used.getToken(2) << "'\n";
   agent->sendMsg(msg.str().c_str());


	return 1;
}

/*******************************************************************************************
 * quitcom - player quits from the game
 *******************************************************************************************/
int quitcom(MUD &engine, Action &act_used) {
   std::shared_ptr<Player> agent = std::dynamic_pointer_cast<Player>(act_used.getAgent());

	if (agent == nullptr) {
		std::string msg("Attempt to call quitcom on non-player agent '");
		msg += act_used.getAgent()->getID();
		msg += "'";
		mudlog->writeLog(msg.c_str());
		return 0;
	}

	EntityDB &edb = *engine.getEntityDB();
	edb.purgeEntity(agent);

	agent->clearNonSaved(false);

	agent->sendMsg("Quitting the game.\nGoodbye!\n");
	agent->quit();
	return 1;

}

