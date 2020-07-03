#include <iostream>
#include <sstream>
#include <memory>
#include "Location.h"
#include "Player.h"
#include "Getable.h"
#include "MUD.h"
#include "misc.h"
#include "global.h"
#include "Door.h"

const char *lflag_list[] = {"outdoors", "bright", "death", "realtime", "nomobiles", "dark", "nosummon", "private", 
									 "oneperson", "noteleport", "peaceful", "maze", "soundproof", NULL};
const char *eflag_list[] = {"hidden", "special", NULL};

const char *exitlist[] = {"north", "south", "east", "west", "up", "down", "northeast", "northwest", 
									"southeast", "southwest", NULL};
Location::exitdirs opposite[] = {Location::South, Location::North, Location::West, Location::East, 
											Location::Down, Location::Up, Location::Southwest, Location::Southeast, 
											Location::Northwest, Location::Northeast, Location::Custom};

/*********************************************************************************************
 * Location (constructor) - 
 *
 *********************************************************************************************/
Location::Location(const char *id):
								Physical(id)
{
	_typename = "Location";

}

// Copy constructor
Location::Location(const Location &copy_from):
								Physical(copy_from)
{

}


Location::~Location() {

}

/*********************************************************************************************
 * saveData - Called by a child class to save Location-specific data into an XML document
 *
 *    Params:  entnode - This entity's node within the XML tree so attributes can be added to
 *                       it. This should be set up by a child class
 *
 *********************************************************************************************/

void Location::saveData(pugi::xml_node &entnode) const {

	// First, call the parent version
	Physical::saveData(entnode);

   // Add organism data (none yet)
   // pugi::xml_attribute idnode = entnode.append_attribute("id");
   // idnode.set_value(_id.c_str());

}

/*********************************************************************************************
 * loadData - Called by a child class to populate Location-specific data from an XML document
 *
 *    Params:  entnode - This entity's node within the XML tree so attributes can be drawn from
 *                       it
 *
 *    Returns: 1 for success, 0 for failure
 *
 *********************************************************************************************/

int Location::loadData(pugi::xml_node &entnode) {

	// First, call the parent function
	int results = 0;
	if ((results = Physical::loadData(entnode)) != 1)
		return results;

	std::stringstream errmsg;

   // Get the acttype - must be either hardcoded or script
	pugi::xml_node node = entnode.child("desc");
   if (node == nullptr) {
      errmsg << "Location '" << getID() << "' missing mandatory desc field.";
      mudlog->writeLog(errmsg.str().c_str());
      return 0;
   }
	setDesc(node.child_value());

   // Get the acttype - must be either hardcoded or script
   pugi::xml_attribute attr = entnode.attribute("title");
   if (node == nullptr) {
      errmsg << "ERROR: Location '" << getID() << "' missing mandatory title field.";
      mudlog->writeLog(errmsg.str().c_str());
      return 0;
   }

	setTitle(attr.value());

   // Get the Exits (if any)
   for (pugi::xml_node exitnode = entnode.child("exit"); exitnode; exitnode = exitnode.next_sibling("exit")) {
		locexit new_exit;

		// Get the direction name such as "east"
      pugi::xml_attribute attr = exitnode.attribute("name");
      if (attr == nullptr) {
         errmsg << "ERROR: Location '" << getID() << "' Exit node missing mandatory name field.";
         mudlog->writeLog(errmsg.str().c_str());
         return 0;
      }		
		new_exit.dir = attr.value();
		
		// Get the ID string of the location to link
      attr = exitnode.attribute("location");
      if (attr == nullptr) {
         errmsg << "ERROR: Location '" << getID() << "' Exit node missing mandatory location field.";
         mudlog->writeLog(errmsg.str().c_str());
         return 0;
      }
      new_exit.link_id = attr.value();

		// Get any exit flags for this exit
		for (pugi::xml_node eflag = exitnode.child("flag"); eflag; eflag = eflag.next_sibling("flag")) {
			attr = eflag.attribute("name");
			if (attr == nullptr) {
				errmsg << "WARNING: Location '" << getID() << "' exit '" << new_exit.dir << 
																			"' flag missing mandatory name field.";
				mudlog->writeLog(errmsg.str().c_str());
				continue;
			}
			new_exit.dir = attr.value();

			// As exit flags are somewhat special, we can't use the normal class find function
		   std::string flagstr = attr.value();
			lower(flagstr);

			size_t i=0;
			while ((eflag_list[i] != NULL) && (flagstr.compare(eflag_list[i]) != 0))
				i++;

			if (eflag_list[i] == NULL) {
            errmsg << "WARNING: Location '" << getID() << "' exit '" << new_exit.dir << "' flag '" << flagstr << 
																		" is not a recognized exit flag.";
            mudlog->writeLog(errmsg.str().c_str());
            continue; 
			}
			new_exit.eflags[i] = true;
		}

		// Get the exit value
		unsigned int i=0;
		new_exit.exitval = Custom;
		while (exitlist[i] != NULL) {
			if (new_exit.dir.compare(exitlist[i]) == 0) {
				new_exit.exitval = (exitdirs) i;
				break;
			}
			i++;
		}
		_exits.push_back(new_exit);
   }

	return 1;
}

/*********************************************************************************************
 * **** functions to set location attributes
 *
 *********************************************************************************************/

void Location::setDesc(const char *newdesc) {
	_desc = newdesc;
}

void Location::setTitle(const char *newtitle) {
   _title = newtitle;
}

/*********************************************************************************************
 * setFlagInternal - given the flag string, first checks the parent for the flag, then checks
 *                   this class' flags
 *
 *
 *********************************************************************************************/

bool Location::setFlagInternal(const char *flagname, bool newval) {
   if (Physical::setFlagInternal(flagname, newval))
      return true;

   std::string flagstr = flagname;
   lower(flagstr);

   size_t i=0;
   while ((lflag_list[i] != NULL) && (flagstr.compare(lflag_list[i]) != 0))
      i++;

   if (lflag_list[i] == NULL)
      return false;

   _locflags[i] = true;
   return true;
}

/*********************************************************************************************
 * isFlagSetInternal - given the flag string, first checks the parent for the flag, then checks
 *                   this class' flags
 *
 *    Params:  flagname - flag to set
 *             results - if found, what the flag is set to
 *
 *    Returns: true if the flag was found, false otherwise
 *
 *********************************************************************************************/

bool Location::isFlagSetInternal(const char *flagname, bool &results) {
   if (Physical::isFlagSetInternal(flagname, results))
      return true;

   std::string flagstr = flagname;
   lower(flagstr);

   size_t i=0;
   while ((lflag_list[i] != NULL) && (flagstr.compare(lflag_list[i]) != 0))
      i++;

   if (lflag_list[i] == NULL)
      return false;

   results =_locflags[i];
   return true;

}

/*********************************************************************************************
 * getExit - given an exit name, such as "east", returns a shared pointer to the new location
 *				 or nullptr if not found. exitname should be all lowercase.
 *	getExitAbbrev - Like getExit, but allows for abbreviated cardinal directions and custom
 *
 *		Params: exitname - for the abbrev version, this gets populated with the full exit string
 *					val - if not null, gets populated with the enum exit value
 *
 *********************************************************************************************/

std::shared_ptr<Physical> Location::getExit(const char *exitname) {
   if (exitname == NULL)
      return nullptr;

   for (unsigned int i=0; i<_exits.size(); i++) {
      if (_exits[i].dir.compare(exitname) == 0)
         return _exits[i].link_loc;
   }
   return nullptr;
}

std::shared_ptr<Physical> Location::getExitAbbrev(std::string &exitname, exitdirs *val) {
	std::string dir = exitname;
	if (dir.size() == 0)
		return nullptr;

	if ((dir.size() == 2) && (dir[0] == 'n')) {
		if (dir[1] == 'w')
			dir = "northwest";
		else if (dir[2] == 'e')
			dir = "northeast";
	} else if ((dir.size() == 2) && (dir[0] == 's')) {
		if (dir[1] == 'w')
			dir = "southwest";
		else if (dir[2] == 'e')
			dir = "southeast";
	}

	// Loop through the existing exits, getting return values for the parameters
	for (unsigned int i=0; i<_exits.size(); i++) {
		if (_exits[i].dir.compare(0, dir.size(), dir) == 0) {
			if (val != NULL)
				*val = _exits[i].exitval;
			exitname = _exits[i].dir;
			return _exits[i].link_loc;
		}
	}
	return nullptr;
}
   //
/*********************************************************************************************
 * addLinks - Adds shared_ptr links between this object and others in the EntityDB. Polymorphic
 *
 *
 *********************************************************************************************/

void Location::addLinks(EntityDB &edb, std::shared_ptr<Physical> self) {
	std::stringstream msg;
	(void) self;

	// Go through the exits, creating links
	auto exit_it = _exits.begin();
	while (exit_it != _exits.end()) {
		std::shared_ptr<Physical> entptr = edb.getPhysical(exit_it->link_id.c_str());
		std::shared_ptr<Location> locptr;
		std::shared_ptr<Door> doorptr;

		if (entptr == nullptr) {
			msg << "WARNING: Location '" << getID() << "' " << exit_it->dir << " exit '" << 
										exit_it->link_id.c_str() << "' doesn't appear to exist.";
			mudlog->writeLog(msg.str().c_str());
			msg.str("");
			exit_it = _exits.erase(exit_it);
			continue;
		} 

		if ((locptr = std::dynamic_pointer_cast<Location>(entptr)) != nullptr) {
         exit_it->link_loc = locptr;
         exit_it++;
		} else if ((doorptr = std::dynamic_pointer_cast<Door>(entptr)) != nullptr) {
			exit_it->link_loc = doorptr;
			exit_it++;
		} else {
         msg << "WARNING: Location '" << getID() << "' exit '" << exit_it->dir << 
																		"' doesn't appear to be a Location or Door class.";
         mudlog->writeLog(msg.str().c_str());
         exit_it = _exits.erase(exit_it);
		}
	}
}


/*********************************************************************************************
 * getExitsStr - Formats a display string of the exits that can be sent to the player
 *
 *		Params:	buf - the buffer where the string is populated
 *	
 *		Returns: pointer to the string in the param buffer
 *
 *********************************************************************************************/

const char *Location::getExitsStr(std::string &buf) {
	buf.clear();

	buf += "Exits:\n";
	std::string dir;
	unsigned int count = 0;
	for (unsigned int i=0; i<_exits.size(); i++) {

		// Skip hidden exits
		if (_exits[i].eflags[(size_t) Hidden])
			continue;

		// Get the exit's location--if it is a door, get the other side
		std::shared_ptr<Physical> locptr = _exits[i].link_loc;
		std::shared_ptr<Door> doorptr = std::dynamic_pointer_cast<Door>(locptr);

		dir = _exits[i].dir;
		dir[0] = toupper(dir[0]);

		// It's a door!
		if (doorptr != nullptr) {
			// The door is open, get the exit
			if (doorptr->getDoorState() == Static::Open) {
				locptr = doorptr->getOppositeLoc(this);

			// The door is closed and not hidden, mark it blocked!
			} else if (!doorptr->isDoorFlagSet(Door::HideClosedExit)) {
				buf += "   ";
				buf += dir;
				buf += ": [Blocked]\n";
				count++;
				continue;

			// The door is closed and hidden, skip it
			} else
				continue;
		}

		buf += "   ";
		buf += dir;
		buf += ": ";
		buf += locptr->getTitle();
		buf += "\n";
		count++;
	}

	if (count == 0) 
		buf += "   None\n";
	return buf.c_str();
}


/*********************************************************************************************
 * listContents - preps a string with a list of visible getables and organisms first
 *
 *
 *********************************************************************************************/

const char *Location::listContents(std::string &buf, const Physical *exclude) const {
	auto cit = _contained.begin();

	// Show doors first
	for ( ; cit != _contained.end(); cit++) {
		std::shared_ptr<Door> dptr = std::dynamic_pointer_cast<Door>(*cit);
		if (dptr == nullptr)
			continue;

		const char *rdesc = dptr->getCurRoomdesc(this);
		if (rdesc != NULL) {
			buf += rdesc;
			buf += "\n";
		}
		
	}	

	// Show getables
	for (cit = _contained.begin(); cit != _contained.end(); cit++) {
      std::shared_ptr<Getable> gptr = std::dynamic_pointer_cast<Getable>(*cit);

      if (gptr == nullptr)
         continue;
	
		buf += gptr->getRoomDesc();
		buf += "\n";	
	}

   // Show organisms next
   for (cit = _contained.begin(); cit != _contained.end(); cit++) {
      std::shared_ptr<Organism> optr = std::dynamic_pointer_cast<Organism>(*cit);
		if (optr == nullptr)
			continue;

		// Skip players on the exclude list
		std::shared_ptr<Player> pptr = std::dynamic_pointer_cast<Player>(optr);
      if ((pptr != nullptr) && (*pptr == exclude))
         continue;

		std::string reviewstr;
		buf += optr->getReviewProcessed(Organism::Standing, reviewstr);
		buf += "\n";
   }
	return buf.c_str();
}

/*********************************************************************************************
 * sendMsg - sends a message to all organisms in this room
 *
 *
 *********************************************************************************************/

void Location::sendMsg(const char *msg, std::shared_ptr<Physical> exclude) {
   std::string unformatted = msg;
   sendMsg(unformatted, exclude);
}

void Location::sendMsg(std::string &msg, std::shared_ptr<Physical> exclude) {
	
	auto cit = _contained.begin();
	for ( ; cit != _contained.end(); cit++) {
		// Compare against exclude - comparing shared pointers wasn't working
		if (*cit != exclude)
			(*cit)->sendMsg(msg);
	}	
}


/*********************************************************************************************
 * getOppositeDir - Given an exit enum, returns the opposing direction
 *
 *********************************************************************************************/

Location::exitdirs Location::getOppositeDir(Location::exitdirs dir) {
	return opposite[dir];
}

/*********************************************************************************************
 * setExit - sets the exit to the parameter passed in (assuming it's a loc or door)
 *
 *********************************************************************************************/

bool Location::setExit(const char *exitname, std::shared_ptr<Physical> new_exit) {
	locexit new_le;

	if ((std::dynamic_pointer_cast<Location>(new_exit) == nullptr) && 
		 (std::dynamic_pointer_cast<Door>(new_exit) == nullptr))
		return false;

	new_le.dir = exitname;
	new_le.link_id = new_exit->getID();
	new_le.link_loc = new_exit;
	
   // Get the exit value
   unsigned int i=0;
   new_le.exitval = Custom;
   while (exitlist[i] != NULL) {
      if (new_le.dir.compare(exitlist[i]) == 0) {
         new_le.exitval = (exitdirs) i;
         break;
      }
      i++;
   }

   _exits.push_back(new_le);
	return true;
}


/*********************************************************************************************
 * clrExit - removes the specified exit - returns true for success, false if the exit is not found
 *
 *********************************************************************************************/

bool Location::clrExit(const char *exitname) {
	for (unsigned int i=0; i<_exits.size(); i++) {
		if (_exits[i].dir.compare(exitname) == 0) {
			_exits.erase(_exits.begin() + i);
			return true;
		}
	}
	return false;
}
