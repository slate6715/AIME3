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

