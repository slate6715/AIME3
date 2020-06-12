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
#include "Door.h"


/*******************************************************************************************
 * infocom - displays an info file to the user 
 *******************************************************************************************/
int infocom(MUD &engine, Action &act_used) {

	std::shared_ptr<Organism> actor = act_used.getActor();


	libconfig::Config &mud_cfg = *(engine.getConfig());

	// First, check to make sure the request uses only letters and/or numbers
	std::regex fncheck("[a-z0-9]+");
	std::string infoname = act_used.getToken(1);
	lower(infoname);
	
	if (!std::regex_match(infoname, fncheck)) {
		actor->sendMsg("The info filename you are seeking can only consist of letters and numbers.\n");
		return 0;		
	}

	std::string fullpath;
	mud_cfg.lookupValue("datadir.infodir", fullpath);

	fullpath += "/";
	fullpath += act_used.getToken(1);
	fullpath += ".info";

	if (!actor->sendFile(fullpath.c_str())) {
		actor->sendMsg("That info file does not appear to exist.\n");
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

	std::shared_ptr<Organism> actor = act_used.getActor();
	std::shared_ptr<Location> cur_loc = std::dynamic_pointer_cast<Location>(actor->getCurLoc());

	if (cur_loc == nullptr) {
		std::stringstream msg;
		msg << "SERIOUS ERROR: gocom function, actor " << actor->getID() << 
					" current location not a Location class or not set.";
		mudlog->writeLog(msg.str().c_str());

		actor->sendMsg("SERIOUS ERROR: You do not have a valid current location. See an Admin.\n");
		return 0;
	}

	Location::exitdirs exitval;
	std::shared_ptr<Entity> exit_loc = cur_loc->getExitAbbrev(dir, &exitval);

	if (exit_loc == nullptr) {
		actor->sendMsg("There is no exit that direction.\n");
		return 0;
	}

	// If the exit is a door, we need to get the opposite side of the door if it's open
	std::shared_ptr<Door> door_ptr = std::dynamic_pointer_cast<Door>(exit_loc);
	if (door_ptr != nullptr) {
		if ((door_ptr->getDoorState() == Door::Closed) || (door_ptr->getDoorState() == Door::Locked)) {
			actor->sendMsg("That way is blocked.\n");
			return 0;
		}

		// Special condition goes here


		exit_loc = door_ptr->getOppositeLoc(actor->getCurLoc());
	}

	std::string reviewstr;
	actor->getReviewProcessed(Organism::Leaving, reviewstr, exitval, dir.c_str());

	cur_loc->sendMsg(reviewstr, actor);
	cur_loc->sendMsg("\n", actor);
	actor->moveEntity(exit_loc);

   actor->getReviewProcessed(Organism::Entering, reviewstr, Location::getOppositeDir(exitval), dir.c_str());
	exit_loc->sendMsg(reviewstr, actor);
	exit_loc->sendMsg("\n");
	actor->sendCurLocation();

	return 1;
}

/*******************************************************************************************
 * lookcom - used to display a room or to examine something
 *******************************************************************************************/
int lookcom(MUD &engine, Action &act_used) {
	std::shared_ptr<Organism> actor = act_used.getActor();
	(void) engine; // Eliminate compile warnings

	// If they just typed look or examine
	if (act_used.numTokens() == 1) {	
		actor->sendCurLocation();
		return 1;
	}

	// They seem to want to look in something?
	std::string preposition = act_used.getToken(1);
	lower(preposition);

	// If they didn't type a preposition, we assume "at"
	if ((preposition.compare("at")) != 0 && (preposition.compare("in") != 0))
		preposition = "at";

	std::shared_ptr<Entity> target1 = act_used.getTarget1();
	// Examine something
	if (preposition.compare("at") == 0) {
		// Display the description of the entity
		actor->sendMsg(target1->getDesc());

		// If we're examining an organism, also display their visible objects
		std::shared_ptr<Organism> optr = std::dynamic_pointer_cast<Organism>(target1);
		if (optr != nullptr) {
			std::string buf;
			actor->sendMsg(target1->listContents(buf));
		}
		return 1;
	} else if (preposition.compare("in") == 0) {
		std::string msg("The "), name;
		msg += act_used.getTarget1()->getNameID(name);
		msg += " contains:\n";
		actor->sendMsg(msg.c_str());

		actor->sendMsg(act_used.getTarget1()->listContents(msg));
		return 1;
	}

	// Look inside something (TBD)

	return 1;
}

/*******************************************************************************************
 * exitscom - used to show all the exits from this location
 *******************************************************************************************/
int exitscom(MUD &engine, Action &act_used) {
   std::shared_ptr<Organism> actor = act_used.getActor();
   (void) engine; // Eliminate compile warnings

	actor->sendMsg("\n");
   actor->sendExits();
	actor->sendMsg("\n");
   return 1;
}

/*******************************************************************************************
 * drocom - drop something from the player's inventory
 *******************************************************************************************/

int dropcom(MUD &engine, Action &act_used) {
   std::shared_ptr<Organism> actor = act_used.getActor();
   (void) engine; // Eliminate compile warnings

	std::shared_ptr<Getable> gptr = std::dynamic_pointer_cast<Getable>(act_used.getTarget1());

	if (gptr == nullptr) {
		actor->sendMsg("You don't seem to have that.\n");
		return 0;
	} else if (gptr->isFlagSet("NoDrop")) {
		actor->sendMsg("You are unable to drop the ");
		actor->sendMsg(gptr->getTitle());
		actor->sendMsg("\n");
		return 0;
	}

	gptr->moveEntity(actor->getCurLoc(), act_used.getTarget1());
	std::string msg("You drop the ");
	msg += gptr->getTitle();
	msg += "\n";
	actor->sendMsg(msg.c_str());

   return 1;
}

/*******************************************************************************************
 * getcom - get something from the ground or a container
 *******************************************************************************************/
int getcom(MUD &engine, Action &act_used) {
   std::shared_ptr<Organism> actor = act_used.getActor();
   (void) engine; // Eliminate compile warnings

   std::shared_ptr<Getable> gptr = std::dynamic_pointer_cast<Getable>(act_used.getTarget1());
   std::shared_ptr<Entity> cur_loc = actor->getCurLoc();

   gptr->moveEntity(actor, act_used.getTarget1());
   std::string msg("You pick up the ");
   msg += gptr->getTitle();
   msg += "\n";
   actor->sendMsg(msg.c_str());

   gptr->popRoomDesc();

   return 1;
}

/*******************************************************************************************
 * inventorycom - display the player's inventory
 *******************************************************************************************/
int inventorycom(MUD &engine, Action &act_used) {
   std::shared_ptr<Organism> actor = act_used.getActor();
   (void) engine; // Eliminate compile warnings

	actor->sendMsg("Inventory contains:\n");

	std::string buf;

	actor->sendMsg(actor->listContents(buf));
	actor->sendMsg("\n");
	return 1;
}

/*******************************************************************************************
 * userscom - shows the users that are currently logged on
 *******************************************************************************************/
int userscom(MUD &engine, Action &act_used) {
	UserMgr &umgr = *engine.getUserMgr();

   std::shared_ptr<Organism> actor = act_used.getActor();

	std::string buf;
	actor->sendMsg(umgr.showUsers(buf));	
	return 1;
}

/*******************************************************************************************
 * saycom - speak to people in the room
 *******************************************************************************************/
int saycom(MUD &engine, Action &act_used) {
	(void) engine;
   std::shared_ptr<Organism> actor = act_used.getActor();

	std::stringstream msg;
	std::string name;
	actor->getNameID(name);
	name[0] = toupper(name[0]);
	
	msg << name << " says '" << act_used.getToken(1) << "'\n";
	actor->getCurLoc()->sendMsg(msg.str().c_str(), actor);

	msg.str("");
	msg << "You say '" << act_used.getToken(1) << "'\n";
	actor->sendMsg(msg.str().c_str());
	return 1;
}

/*******************************************************************************************
 * chatcom - speak to people in the mud who have joined the chat channel
 *******************************************************************************************/
int chatcom(MUD &engine, Action &act_used) {
   std::shared_ptr<Organism> actor = act_used.getActor();
	UserMgr &umgr = *engine.getUserMgr();

   std::stringstream msg;
   std::string name;
   actor->getNameID(name);
   name[0] = toupper(name[0]);

	std::vector<std::string> ex_flags;
	ex_flags.push_back("nochat");

   msg << name << " chats '" << act_used.getToken(1) << "'\n";

	// Send this to everyone who does not have nochat set and is not the actor
	umgr.sendMsg(msg.str().c_str(), &ex_flags, NULL, actor);

   msg.str("");
   msg << "You chat '" << act_used.getToken(1) << "'\n";
   actor->sendMsg(msg.str().c_str());

	return 1;
}

/*******************************************************************************************
 * tellcom - tell something to a specific player
 *******************************************************************************************/
int tellcom(MUD &engine, Action &act_used) {
	(void) engine;

   std::shared_ptr<Organism> actor = act_used.getActor();
	std::string pname, tname;
	std::stringstream msg;

	actor->getNameID(pname);
	act_used.getTarget1()->getNameID(tname);
	pname[0] = toupper(pname[0]);
	tname[0] = toupper(tname[0]);
   msg << pname << " tells you '" << act_used.getToken(2) << "'\n";

   // Send this to everyone who does not have nochat set and is not the actor
   act_used.getTarget1()->sendMsg(msg.str().c_str());

   msg.str("");
   msg << "You tell " << tname << " '" << act_used.getToken(2) << "'\n";
   actor->sendMsg(msg.str().c_str());


	return 1;
}

/*******************************************************************************************
 * quitcom - player quits from the game
 *******************************************************************************************/
int quitcom(MUD &engine, Action &act_used) {
   std::shared_ptr<Player> actor = std::dynamic_pointer_cast<Player>(act_used.getActor());

	if (actor == nullptr) {
		std::string msg("Attempt to call quitcom on non-player actor '");
		msg += act_used.getActor()->getID();
		msg += "'";
		mudlog->writeLog(msg.c_str());
		return 0;
	}

	EntityDB &edb = *engine.getEntityDB();
	edb.purgeEntity(actor);

	actor->clearNonSaved(false);

	actor->sendMsg("Quitting the game.\nGoodbye!\n");
	actor->quit();
	return 1;

}

/*******************************************************************************************
 * opencom - open something, such as a door or container
 *******************************************************************************************/

int opencom(MUD &engine, Action &act_used) {
   std::shared_ptr<Organism> actor = act_used.getActor();
	std::stringstream msg;
   (void) engine; // Eliminate compile warnings

	std::shared_ptr<Entity> target = act_used.getTarget1();
	std::string errmsg;
	if (!target->open(errmsg)) {
		actor->sendMsg(errmsg.c_str());
		return 0;
	}

	msg << "You open the " << target->getTitle() << ".\n";
	actor->sendMsg(msg.str().c_str());
	msg.str("");
	
	msg << actor->getTitle() << " opens the " << target->getTitle() << ".\n";
	actor->getCurLoc()->sendMsg(msg.str().c_str(), actor);

   return 1;
}

/*******************************************************************************************
 * closecom - close something, such as a door or container
 *******************************************************************************************/

int closecom(MUD &engine, Action &act_used) {
   std::shared_ptr<Organism> actor = act_used.getActor();
   std::stringstream msg;
   (void) engine; // Eliminate compile warnings

   std::shared_ptr<Entity> target = act_used.getTarget1();
   std::string errmsg;
   if (!target->close(errmsg)) {
      actor->sendMsg(errmsg.c_str());
      return 0;
   }

   msg << "You close the " << target->getTitle() << ".\n";
   actor->sendMsg(msg.str().c_str());

	msg.str("");
   msg << actor->getTitle() << " closes the " << target->getTitle() << ".\n";
   actor->getCurLoc()->sendMsg(msg.str().c_str(), actor);

   return 1;
}

/*******************************************************************************************
 * statscom - shows the user their stats
 *******************************************************************************************/
int statscom(MUD &engine, Action &act_used) {
	(void) engine;
	std::stringstream msg;

   std::shared_ptr<Organism> actor = act_used.getActor();

	msg << "\n&+cPlayer stats:\n---------------------------------------------\n&*";
	msg << "&+yStrength:&* " << actor->getAttribInt("strength") << "\n";
	msg << "&+yDamage:&* " << actor->getAttribInt("damage") << "\n";

	msg << "\n&+MTraits:&*\n";
	actor->sendMsg(msg.str().c_str());

	actor->sendTraits();

	actor->sendMsg("&+c---------------------------------------------\n\n&*");

   return 1;
}

