#include <iostream>
#include <libconfig.h++>
#include <regex>
#include <memory>
#include <sstream>
#include "actions.h"
#include "MUD.h"
#include "Action.h"
#include "misc.h"
#include "Location.h"
#include "global.h"
#include "Getable.h"
#include "Door.h"
#include "Equipment.h"


/*******************************************************************************************
 * infocom - displays an info file to the user 
 *******************************************************************************************/
int infocom(MUD &engine, Action &act_used) {

	std::shared_ptr<Organism> actor = act_used.getActor();


	libconfig::Config &mud_cfg = *(engine.getConfig());

	// First, check to make sure the request uses only letters and/or numbers
	std::regex fncheck("[a-z0-9]+");
	std::string infoname = act_used.getToken(0);
	lower(infoname);
	
	if (!std::regex_match(infoname, fncheck)) {
		actor->sendMsg("The info filename you are seeking can only consist of letters and numbers.\n");
		return 0;		
	}

	std::string fullpath;
	mud_cfg.lookupValue("datadir.infodir", fullpath);

	fullpath += "/";
	fullpath += act_used.getToken(0);
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
	std::shared_ptr<Physical> exit_loc = cur_loc->getExitAbbrev(dir, &exitval);

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

   // Look for onexit specials in the location
   std::vector<std::pair<std::string, std::shared_ptr<Physical>>> variables;
	std::vector<std::pair<std::string, std::string>> variable_strs;

	variables.push_back(std::pair<std::string, std::shared_ptr<Physical>>("actor", actor));
	variables.push_back(std::pair<std::string, std::shared_ptr<Physical>>("toloc", exit_loc));
	variables.push_back(std::pair<std::string, std::shared_ptr<Physical>>("fromloc", cur_loc));

	variable_strs.push_back(std::pair<std::string, std::string>("exit", dir.c_str()));

	int results;
   if ((results = cur_loc->execSpecial("onexit", variables, NULL, NULL, &variable_strs)) == 2) {
      return 0;
   }

	std::string reviewstr;
	actor->getReviewProcessed(Organism::Leaving, reviewstr, exitval, dir.c_str());

	cur_loc->sendMsg(reviewstr, actor);
	cur_loc->sendMsg("\n", actor);
	actor->movePhysical(exit_loc);

   actor->getReviewProcessed(Organism::Entering, reviewstr, Location::getOppositeDir(exitval), dir.c_str());
	exit_loc->sendMsg(reviewstr, actor);
	exit_loc->sendMsg("\n");
	actor->sendCurLocation();

   if ((results = exit_loc->execSpecial("onentry", variables, NULL, NULL, &variable_strs)) == 2) {
      return 0;
   }

	if (exit_loc->isFlagSet("Death")) {
		actor->sendMsg("You have died.\n");
		actor->kill();
	}

	return 1;
}

/*******************************************************************************************
 * lookcom - used to display a room or to examine something
 *******************************************************************************************/
int lookcom(MUD &engine, Action &act_used) {
	std::shared_ptr<Organism> actor = act_used.getActor();
	(void) engine; // Eliminate compile warnings

	// If they just typed look or examine
	if (act_used.numTokens() == 0) {	
		actor->sendCurLocation();
		return 1;
	}

	// They seem to want to look in something?
	std::string preposition = act_used.getToken(0);
	lower(preposition);

	// If they didn't type a preposition, we assume "at"
	if ((preposition.compare("at")) != 0 && (preposition.compare("in") != 0))
		preposition = "at";

	std::shared_ptr<Physical> target1 = act_used.getTarget1();
	// Examine something
	if (preposition.compare("at") == 0) {
		// Display the description of the entity
		actor->sendMsg(target1->getExamine());

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

	std::stringstream msg;
	std::string buf;
	gptr->movePhysical(actor->getCurLoc(), act_used.getTarget1());
	msg << "You drop the " << gptr->getTitle() << "\n";
	actor->sendMsg(msg.str().c_str());

	msg.str("");
   msg << actor->getGameName(buf) << " drops the " << gptr->getGameName(buf) << ".\n";
   actor->getCurLoc()->sendMsg(msg.str().c_str(), actor);

   return 1;
}

/*******************************************************************************************
 * eatcom - consume something in the player's inventory
 *******************************************************************************************/

int eatcom(MUD &engine, Action &act_used) {
   std::shared_ptr<Organism> actor = act_used.getActor();
   (void) engine; // Eliminate compile warnings

   std::shared_ptr<Getable> gptr = std::dynamic_pointer_cast<Getable>(act_used.getTarget1());

   if (gptr == nullptr) {
      actor->sendMsg("You don't seem to have that.\n");
      return 0;
   } 

	if (!gptr->isFlagSet("Food")) {
      actor->sendMsg("That is not edible.\n");
      return 0;
   }

	if (gptr->hasAttribute("heal")) {
		int heal = gptr->getAttribInt("heal");
		actor->incrAttribute("health", heal);
	}

	std::stringstream msg;
	std::string buf;
	msg << "You greedily consume the " << gptr->getGameName(buf) << ".\n";
	actor->sendMsg(msg.str().c_str());

	msg.str("");
	msg << actor->getGameName(buf) << " greedily consumes the " << gptr->getGameName(buf) << ".\n";
	actor->getCurLoc()->sendMsg(msg.str().c_str(), actor);

	actor->removePhysical(gptr);

   return 1;
}

/*******************************************************************************************
 * getcom - get something from the ground or a container
 *******************************************************************************************/
int getcom(MUD &engine, Action &act_used) {
   std::shared_ptr<Organism> actor = act_used.getActor();
   std::shared_ptr<Physical> cur_loc = actor->getCurLoc();
	std::shared_ptr<Physical> pptr;
	std::shared_ptr<Getable> gptr;
	std::string buf;
	bool in_inventory = true;
   (void) engine; // Eliminate compile warnings

	// Need to find the objects ourselves. If only one token, then no container involved
	std::stringstream msg;
	if (act_used.numTokens() == 1) {
		if ((pptr = cur_loc->getContainedByName(act_used.getToken(0))) == nullptr) {
			actor->sendMsg("You cannot find that here.\n");
			return 0;
		} 

		if ((gptr = std::dynamic_pointer_cast<Getable>(pptr)) == nullptr) {
			actor->sendMsg("That is not something you can pick up.\n");
			return 0;
		}

		if (gptr->isGetableFlagSet(Getable::NoGet)) {
			actor->sendMsg("You are unable to get it to budge.");
			return 0;
		}

		gptr->movePhysical(actor, gptr);
		msg << "You pick up the " << gptr->getGameName(buf) << ".\n";
		actor->sendMsg(msg.str().c_str());
		msg.str("");

		msg << actor->getGameName(buf) << " picks up the " << gptr->getGameName(buf) << ".\n";
		cur_loc->sendMsg(msg.str().c_str(), actor);

		gptr->changeRoomDesc(Getable::Dropped);

		return 1;
	}

	// If we're dealing with a container, handle it
	std::string container;
	if (act_used.numTokens() == 2)
		container = act_used.getToken(1);
	else
		container = act_used.getToken(2);

	if ((pptr = actor->getContainedByName(container.c_str())) == nullptr) {
		in_inventory = false;
		if ((pptr = cur_loc->getContainedByName(container.c_str())) == nullptr) {
			msg << "You cannot find the " << container << " here.\n";
			actor->sendMsg(msg.str().c_str());
			return 0;
		}
	}

	
   std::shared_ptr<Static> sptr = std::dynamic_pointer_cast<Static>(pptr);
	if ((sptr == nullptr) || (!sptr->isStaticFlagSet(Static::Container))) {
		msg << "The " << sptr->getGameName(buf) << " is not a container.\n";
		actor->sendMsg(msg.str().c_str());
		return 0; 
	}

	if (sptr->getDoorState() != Static::Open) {
		msg << "The " << sptr->getGameName(buf) << " is closed.\n";
		actor->sendMsg(msg.str().c_str());
		return 0;
	}

	std::shared_ptr<Physical> pptr2 = sptr->getContainedByName(act_used.getToken(0));
	if ((pptr2 == nullptr) || ((gptr = std::dynamic_pointer_cast<Getable>(pptr2)) == nullptr)) {
		msg << "There is no getable object named '" << act_used.getToken(0) << "' in the " << sptr->getGameName(buf) << "\n";
		actor->sendMsg(msg.str().c_str());
		return 0;
	}

   gptr->movePhysical(actor, gptr);

	msg << "You pull the " << gptr->getGameName(buf) << " out of the " << sptr->getGameName(buf) << ".\n";
	actor->sendMsg(msg.str().c_str());

	if (!in_inventory) {
		msg.str("");
		msg << actor->getTitle() << " pulls something out of the " << sptr->getGameName(buf) << ".\n";
		cur_loc->sendMsg(msg.str().c_str(), actor);
	}

	// If it is lit, we might want the lit review
   if (gptr->isStaticFlagSet(Static::Lit)) {
      if (gptr->changeRoomDesc(Getable::Lit))
         return 1;
   }

   gptr->changeRoomDesc(Getable::Dropped);

   return 1;
}

/*******************************************************************************************
 * putcom - put something in a container
 *******************************************************************************************/
int putcom(MUD &engine, Action &act_used) {
   std::shared_ptr<Organism> actor = act_used.getActor();
   std::shared_ptr<Physical> cur_loc = actor->getCurLoc();
   std::shared_ptr<Physical> pptr;
   std::shared_ptr<Getable> gptr;
   std::string buf;
	std::stringstream msg;
   bool in_inventory = true;
   (void) engine; // Eliminate compile warnings

	// First, get the item we want to put in a container
	if ((pptr = actor->getContainedByName(act_used.getToken(0))) == nullptr) {
		if ((pptr = cur_loc->getContainedByName(act_used.getToken(0))) == nullptr) {
         msg << "You cannot find the " << act_used.getToken(0) << " here.\n";
         actor->sendMsg(msg.str().c_str());
         return 0;
		}
	}

	if ((gptr = std::dynamic_pointer_cast<Getable>(pptr)) == nullptr) {
		msg << "You cannot pick up the " << pptr->getTitle() << ".\n";
		actor->sendMsg(msg.str().c_str());
		return 0;
	}

   // If we're dealing with a container, handle it
   std::string container;
   if (act_used.numTokens() == 2)
      container = act_used.getToken(1);
   else
      container = act_used.getToken(2);

   if ((pptr = actor->getContainedByName(container.c_str())) == nullptr) {
      in_inventory = false;
      if ((pptr = cur_loc->getContainedByName(container.c_str())) == nullptr) {
         msg << "You cannot find the " << container << " here.\n";
         actor->sendMsg(msg.str().c_str());
         return 0;
      }
   }

   std::shared_ptr<Static> sptr = std::dynamic_pointer_cast<Static>(pptr);
   if ((sptr == nullptr) || (!sptr->isStaticFlagSet(Static::Container))) {
      msg << "The " << sptr->getGameName(buf) << " is not a container.\n";
      actor->sendMsg(msg.str().c_str());
      return 0;
   }

   if (sptr->getDoorState() != Static::Open) {
      msg << "The " << sptr->getGameName(buf) << " is closed.\n";
      actor->sendMsg(msg.str().c_str());
      return 0;
   }

   gptr->movePhysical(sptr, gptr);

   msg << "You put the " << gptr->getTitle() << " in the " << sptr->getTitle() << ".\n";
   actor->sendMsg(msg.str().c_str());

   if (!in_inventory) {
      msg.str("");
      msg << actor->getTitle() << " puts something into the " << sptr->getGameName(buf) << ".\n";
      cur_loc->sendMsg(msg.str().c_str(), actor);
   }

   gptr->changeRoomDesc(Getable::Dropped);

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
	
	msg << name << " says '" << act_used.getToken(0) << "'\n";
	actor->getCurLoc()->sendMsg(msg.str().c_str(), actor);

	msg.str("");
	msg << "You say '" << act_used.getToken(0) << "'\n";
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

   msg << name << " chats '" << act_used.getToken(0) << "'\n";

	// Send this to everyone who does not have nochat set and is not the actor
	umgr.sendMsg(msg.str().c_str(), &ex_flags, NULL, actor);

   msg.str("");
   msg << "You chat '" << act_used.getToken(0) << "'\n";
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
   msg << pname << " tells you '" << act_used.getToken(1) << "'\n";

   // Send this to everyone who does not have nochat set and is not the actor
   act_used.getTarget1()->sendMsg(msg.str().c_str());

   msg.str("");
   msg << "You tell " << tname << " '" << act_used.getToken(1) << "'\n";
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
	edb.purgePhysical(actor);

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

	std::shared_ptr<Physical> target = act_used.getTarget1();
	std::string errmsg;
	if (!target->open(errmsg)) {
		actor->sendMsg(errmsg.c_str());
		return 0;
	}

	msg << "You open the " << target->getTitle() << ".\n";
	actor->sendMsg(msg.str().c_str());
	msg.str("");

	std::string buf;	
	msg << actor->getGameName(buf) << " opens the " << target->getTitle() << ".\n";
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

   std::shared_ptr<Physical> target = act_used.getTarget1();
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

/*******************************************************************************************
 * equipcom - Wear or wield equipment
 *******************************************************************************************/

int equipcom(MUD &engine, Action &act_used) {
   std::shared_ptr<Organism> actor = act_used.getActor();
   (void) engine; // Eliminate compile warnings

   std::shared_ptr<Equipment> eptr = std::dynamic_pointer_cast<Equipment>(act_used.getTarget1());

   if (eptr == nullptr) {
      actor->sendMsg("You can't equip that item.\n");
      return 0;
	}

	std::string errmsg;
	if (!actor->equip(eptr, errmsg)) {
		actor->sendMsg(errmsg.c_str());
		return 0;
	}

   std::string msg("You equip the ");
   msg += eptr->getTitle();
   msg += "\n";
   actor->sendMsg(msg.c_str());

   return 1;
}

/*******************************************************************************************
 * removecom - Unwear or unwield equipment
 *******************************************************************************************/

int removecom(MUD &engine, Action &act_used) {
   std::shared_ptr<Organism> actor = act_used.getActor();
   (void) engine; // Eliminate compile warnings

   std::shared_ptr<Equipment> eptr = std::dynamic_pointer_cast<Equipment>(act_used.getTarget1());

   if (eptr == nullptr) {
      actor->sendMsg("You do not have that equipped.\n");
      return 0;
   }

   std::string errmsg;
   if (!actor->remove(eptr, errmsg)) {
      actor->sendMsg(errmsg.c_str());
      return 0;
   }

   std::string msg("You remove the ");
   msg += eptr->getTitle();
   msg += "\n";
   actor->sendMsg(msg.c_str());

   return 1;
}

/*******************************************************************************************
 * tiecom - tie a rope to something, usually a door
 *******************************************************************************************/
int tiecom(MUD &engine, Action &act_used) {
	std::stringstream msg;

   std::shared_ptr<Organism> actor = act_used.getActor();
   (void) engine; // Eliminate compile warnings

   std::shared_ptr<Getable> gptr = std::dynamic_pointer_cast<Getable>(act_used.getTarget1());
   std::shared_ptr<Physical> cur_loc = actor->getCurLoc();

	if (!gptr->isGetableFlagSet(Getable::Rope)) {
		actor->sendMsg("That cannot be tied to anything.\n");
		return 0;
	}

	std::shared_ptr<Door> dptr = std::dynamic_pointer_cast<Door>(act_used.getTarget2());
	if ((dptr == nullptr) || (!dptr->isDoorFlagSet(Door::RopeDoor))) {
		msg << "You cannot tie the " << gptr->getTitle() << " to anything.\n";
		actor->sendMsg(msg.str().c_str());
		return 0;
	}

	std::string buf;	
	if (!dptr->open(gptr, buf)) {
		actor->sendMsg(buf.c_str());
		return 0;	
	}

	msg << "You tie the " << gptr->getTitle() << " to the " << dptr->getTitle() << ".\n";
	actor->sendMsg(msg.str().c_str());

	msg.str("");
	msg << actor->getTitle() << " ties a " << gptr->getTitle() << " to the " << dptr->getTitle() << ".\n";
	cur_loc->sendMsg(msg.str().c_str(), actor);
	

   return 1;
}

/*******************************************************************************************
 * untiecom - tie a rope to something, usually a door
 *******************************************************************************************/
int untiecom(MUD &engine, Action &act_used) {
   std::stringstream msg;

   std::shared_ptr<Organism> actor = act_used.getActor();
   (void) engine;

	// We need to locate these objects manually. If they did not specify a door, find the first door
	
   std::shared_ptr<Physical> cur_loc = actor->getCurLoc();

	// If there are more than two tokens, they specified an object
	std::shared_ptr<Door> dptr = nullptr;
	if (act_used.numTokens() > 1) {
		std::string doorname;
		if (act_used.numTokens() == 2)
			doorname = act_used.getToken(1);
		else
			doorname = act_used.getToken(2);
		std::shared_ptr<Physical> pptr = cur_loc->getContainedByName(doorname.c_str());
		if (pptr == nullptr) {
			actor->sendMsg("I cannot locate the ");
			actor->sendMsg(doorname);
			actor->sendMsg(" here\n");
			return 0;
		}

		if ((dptr = std::dynamic_pointer_cast<Door>(pptr)) == nullptr) {
			actor->sendMsg("You cannot untie things from that.\n");
			return 0;
		}
	} else {
		auto cont_it = cur_loc->beginContained();
		for (; cont_it != cur_loc->endContained(); cont_it++) {
			if ((dptr = std::dynamic_pointer_cast<Door>(*cont_it)) != nullptr)
				break;
		}

		if (dptr == nullptr) {
			actor->sendMsg("I do not see a ");
			actor->sendMsg(act_used.getToken(0));
			actor->sendMsg(" that can be untied.\n");
			return 0;
		}
	}

	// We should have a door pointer now, try to untie it
	std::string buf;
	std::shared_ptr<Physical> rope = dptr->closeTool(buf);
	if (rope == nullptr) {
		actor->sendMsg(buf.c_str());
		return 0;
	}

	rope->movePhysical(actor, rope);

   msg << "You untie the " << rope->getTitle() << " from the " << dptr->getTitle() << "\n";
   actor->sendMsg(msg.str().c_str());

   msg.str("");
   msg << actor->getTitle() << " unties the " << rope->getTitle() << " from the " << dptr->getTitle() << ".\n";
   cur_loc->sendMsg(msg.str().c_str(), actor);


   return 1;
}

/*******************************************************************************************
 * failcom - standard function to use as default for mostly specials-only functions
 *******************************************************************************************/

int failcom(MUD &engine, Action &act_used) {
   std::shared_ptr<Organism> actor = act_used.getActor();
   (void) engine; // Eliminate compile warnings

	std::string buf;
	std::stringstream msg;

	if (act_used.getTarget1() != nullptr)
		msg << "You cannot " << act_used.getNameID(buf) << " that.\n";
	else
		msg << "You cannot " << act_used.getNameID(buf) << " here.\n";

	actor->sendMsg(msg.str().c_str());

	return 0;
}

/*******************************************************************************************
 * gotocom - wizard-level function to go to an object or location
 *******************************************************************************************/

int gotocom(MUD &engine, Action &act_used) {
	(void) engine;
	
	std::stringstream msg;
	std::string buf;
	
	std::shared_ptr<Organism> actor = act_used.getActor();
	std::shared_ptr<Location> cur_loc, new_loc;

	std::shared_ptr<Physical> target = act_used.getTarget1();

	cur_loc = std::dynamic_pointer_cast<Location>(actor->getCurLoc());

	// Keep getting cur_loc till we find a location
	while ((target != nullptr) && ((new_loc = std::dynamic_pointer_cast<Location>(target)) == nullptr)) {
		target = target->getCurLoc();
	}

	if (target == nullptr) {
		actor->sendMsg("That object is not contained in a Location.\n");
		return 0;
	}

	msg << actor->getGameName(buf) << " disappears suddenly with a loud pop.\n";
   cur_loc->sendMsg(msg.str().c_str(), actor);
	msg.str("");

	if (target != act_used.getTarget1())
	{
		msg << "You teleport instantly to " << act_used.getTarget1()->getID() << " at " << target->getID() << ".\n";
	}
	else
		msg << "You teleport instantly to " << target->getID() << ".\n";
	actor->sendMsg(msg.str().c_str());

	actor->movePhysical(new_loc, actor);

	msg.str("");
	msg << actor->getGameName(buf) << " appears out of nowhere with a loud pop.\n";
	new_loc->sendMsg(msg.str().c_str(), actor);

	actor->sendCurLocation();

	return 1;	
}

/*******************************************************************************************
 * killcom - start attacking a target
 *******************************************************************************************/

int killcom(MUD &engine, Action &act_used) {
   (void) engine;

   std::stringstream msg;
   std::string buf;

   std::shared_ptr<Organism> actor = act_used.getActor();
   std::shared_ptr<Physical> cur_loc = actor->getCurLoc();

   std::shared_ptr<Organism> target = std::dynamic_pointer_cast<Organism>(act_used.getTarget1());

	if (target == nullptr) {
		actor->sendMsg("You can't kill inanimate objects.\n");
		return 0;
	}
	
	msg << "You extend your fingers towards " << target->getGameName(buf) << 
													", incinerating them with blue lightning and leaving their stuff to loot.\n";
	actor->sendMsg(msg.str().c_str());

	msg.str("");

	msg << actor->getGameName(buf) << " extends their fingers towards you and the last thing you see is &+Cblue"
								" lightning&* flying at you.\n";
	target->sendMsg(msg.str().c_str());

	target->kill();		

	msg.str("");
	msg << actor->getGameName(buf) << " extends their fingers towards " << target->getGameName(buf) <<
						". &+CBlue lightning&* flies from them, incinerating " << target->getGameName(buf) << "!\n";
	cur_loc->sendMsg(msg.str().c_str(), actor);
	return 1;
}


/*******************************************************************************************
 * lightcom - ignite a target that is lightable
 *******************************************************************************************/

int lightcom(MUD &engine, Action &act_used) {
   (void) engine;

   std::stringstream msg;
   std::string buf;

   std::shared_ptr<Organism> actor = act_used.getActor();
   std::shared_ptr<Physical> cur_loc = actor->getCurLoc();

	std::shared_ptr<Static> sptr = std::dynamic_pointer_cast<Static>(act_used.getTarget1());

	std::shared_ptr<Physical> light_from = act_used.getTarget2();

	// If the user didn't specify what to light from, find something in inventory or the room
	if (light_from == nullptr) {
		if (sptr == nullptr) { 
			actor->sendMsg("That cannot be lit.\n");
			return 0;
		}

		std::vector<std::string> flags;
		flags.push_back("canlight");
		flags.push_back("lit");

		// The player needs to have something capable of lighting in their inventory or be lighting
		// something in their inventory and have something in the room
		if ((!actor->containsPhysical(sptr)) && ((light_from = actor->containsFlags(flags)) == nullptr)) {
			actor->sendMsg("You need to be holding either the item to be lit or something to light it from (or both).\n");
			return 0;
		}

		if (light_from == nullptr) {
			if ((light_from = cur_loc->containsFlags(flags)) == nullptr) {
				actor->sendMsg("I see nothing to light it from.\n");
				return 0;
			}
		}
	}

	// Do a quick check to make sure at least one is carried
	if (!actor->containsPhysical(sptr) || (!actor->containsPhysical(light_from))) {
		actor->sendMsg("You need to be holding either the item to be lit or something to light it from (or both).\n");
		return 0;
	}
	
	std::string errmsg;
	if (!sptr->light(errmsg)) {
		actor->sendMsg(errmsg.c_str());
		return 0;
	}
	
	msg << "You ignite the " << sptr->getGameName(buf) << " with the " << light_from->getGameName(buf) << ".\n";
	actor->sendMsg(msg.str().c_str());

	msg.str("");
	msg << actor->getGameName(buf) << " ignites the " << sptr->getGameName(buf) << " with the " << light_from->getGameName(buf) << ".\n";
	cur_loc->sendMsg(msg.str().c_str(), actor);

	return 1;
}

/*******************************************************************************************
 * extinguishcom - puts out a lit object
 *******************************************************************************************/

int extinguishcom(MUD &engine, Action &act_used) {
   (void) engine;

   std::stringstream msg;
   std::string buf;

   std::shared_ptr<Organism> actor = act_used.getActor();
   std::shared_ptr<Physical> cur_loc = actor->getCurLoc();

   std::shared_ptr<Static> sptr = std::dynamic_pointer_cast<Static>(act_used.getTarget1());

   if (sptr == nullptr) {
      actor->sendMsg("That is not lit.\n");
      return 0;
   }

	std::string errmsg;
   if (!sptr->extinguish(errmsg)) {
		actor->sendMsg(errmsg);
	}

   msg << "You extinguish the " << sptr->getGameName(buf) << ".\n";
   actor->sendMsg(msg.str().c_str());

   msg.str("");
   msg << actor->getGameName(buf) << " extinguishes the " << sptr->getGameName(buf) << ".\n";
   cur_loc->sendMsg(msg.str().c_str(), actor);
   return 1;
}

/*******************************************************************************************
 * summoncom - wizard-level function to summon an object or organism
 *******************************************************************************************/

int summoncom(MUD &engine, Action &act_used) {
   (void) engine;

   std::stringstream msg;
   std::string buf;

   std::shared_ptr<Organism> actor = act_used.getActor();
   std::shared_ptr<Location> cur_loc;

   std::shared_ptr<Physical> target = act_used.getTarget1();
	std::shared_ptr<Physical> target_cont = target->getCurLoc();

   cur_loc = std::dynamic_pointer_cast<Location>(actor->getCurLoc());

	// First, take care of moving the object and cur_loc messages
	// If it is a getable object, place it in the actor's inventory
	if ((std::dynamic_pointer_cast<Getable>(target) != nullptr) && (!target->isFlagSet("NoGet"))) {
		target->movePhysical(actor);
		msg << "The " << target->getGameName(buf) << " soars through air, landing in your open hand.\n";
		actor->sendMsg(msg.str().c_str());
		
		msg.str("");
		msg << "The " << target->getGameName(buf) << " soars through the air, landing in the open palm of " << 
																							actor->getGameName(buf) << ".\n";
		cur_loc->sendMsg(msg.str().c_str(), actor);

	// Statics go in the location
	} else if (std::dynamic_pointer_cast<Static>(target) != nullptr) {
      target->movePhysical(actor);
      msg << "The " << target->getGameName(buf) << " soars through the air, landing on the ground with a loud thump.\n";
      cur_loc->sendMsg(msg.str().c_str());

	// Organisms also go in the location, but treat them a bit different
	} else if (std::dynamic_pointer_cast<Organism>(target) != nullptr) {
      msg.str("");
      msg << target->getGameName(buf) << " soars through the air, landing on the ground with a thump "
																		"and a surprised look.\n";
      cur_loc->sendMsg(msg.str().c_str());
      target->movePhysical(actor);

		target->sendMsg("You suddenly soar through the air, landing in a new location!\n");
	}

	// Now take care of some destination messages	
	std::shared_ptr<Location> target_loc = std::dynamic_pointer_cast<Location>(target_cont);

	if (target_loc == nullptr) {
		std::shared_ptr<Organism> target_org = std::dynamic_pointer_cast<Organism>(target_cont);
		if (target_org == nullptr)
			return 1;	// No messages to send if not in the loc or carried by someone

		msg << "The " << target->getGameName(buf) << " flies out of your hands, soaring away into the distance.\n";
		target_org->sendMsg(msg.str().c_str());
		return 1;
	}

	msg << "The " << target->getGameName(buf) << " suddenly soars away into the distance.\n";
	target_loc->sendMsg(msg.str().c_str());
	return 1;
}


/*******************************************************************************************
 * hitcom - strike an object or organism
 *******************************************************************************************/
int hitcom(MUD &engine, Action &act_used) {
   std::stringstream msg;
	std::string buf;

   std::shared_ptr<Organism> actor = act_used.getActor();
   (void) engine; // Eliminate compile warnings

   std::shared_ptr<Organism> optr = std::dynamic_pointer_cast<Organism>(act_used.getTarget1());

	if (optr != nullptr) {
		if (act_used.getTarget2() != nullptr) {
			msg << "You strike " << optr->getGameName(buf) << " hard with the " << act_used.getTarget2()->getGameName(buf) << "! They look like they're about to attack, but then realize combat isn't coded yet. They sigh and shrug.\n";
			actor->sendMsg(msg.str().c_str());
		} else { 
         msg << "You strike " << optr->getGameName(buf) << " hard with your fists! They look like they're about to attack, but then realize combat isn't coded yet. They sigh and shrug.\n";
         actor->sendMsg(msg.str().c_str());

		}
		return 1;
	}

	if (act_used.getTarget2() != nullptr) {
		msg << "You strike the " << act_used.getTarget1()->getGameName(buf) << " hard with the " <<
							act_used.getTarget2()->getGameName(buf) << " but nothing happens.\n";
		actor->sendMsg(msg.str().c_str());
	} else {
      msg << "You strike the " << act_used.getTarget1()->getGameName(buf) << 
															" hard with your fists but nothing happens.\n";
      actor->sendMsg(msg.str().c_str());
	}
	return 1;
}

