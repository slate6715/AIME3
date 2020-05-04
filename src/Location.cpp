#include <iostream>
#include <sstream>
#include <memory>
#include "Location.h"
#include "Getable.h"
#include "MUD.h"
#include "misc.h"
#include "global.h"

const char *lflag_list[] = {"outdoors", "bright", "death", NULL};
const char *eflag_list[] = {"hidden", "special", NULL};


/*********************************************************************************************
 * Location (constructor) - 
 *
 *********************************************************************************************/
Location::Location(const char *id):
								Entity(id)
{


}

// Copy constructor
Location::Location(const Location &copy_from):
								Entity(copy_from)
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
	Entity::saveData(entnode);

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
	if ((results = Entity::loadData(entnode)) != 1)
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
      errmsg << "Location '" << getID() << "' missing mandatory title field.";
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
         errmsg << "Location '" << getID() << "' Exit node missing mandatory name field.";
         mudlog->writeLog(errmsg.str().c_str());
         return 0;
      }		
		new_exit.dir = attr.value();
		
		// Get the ID string of the location to link
      attr = exitnode.attribute("location");
      if (attr == nullptr) {
         errmsg << "Location '" << getID() << "' Exit node missing mandatory location field.";
         mudlog->writeLog(errmsg.str().c_str());
         return 0;
      }
      new_exit.link_id = attr.value();

		// Get any exit flags for this exit
		for (pugi::xml_node eflag = exitnode.child("flag"); eflag; eflag = eflag.next_sibling("flag")) {
			attr = eflag.attribute("name");
			if (attr == nullptr) {
				errmsg << "Location '" << getID() << "' exit '" << new_exit.dir << "' flag missing mandatory name field.";
				mudlog->writeLog(errmsg.str().c_str());
				return 0;
			}
			new_exit.dir = attr.value();

			// As exit flags are somewhat special, we can't use the normal class find function
		   std::string flagstr = attr.value();
			lower(flagstr);

			size_t i=0;
			while ((eflag_list[i] != NULL) && (flagstr.compare(eflag_list[i]) != 0))
				i++;

			if (eflag_list[i] == NULL) {
            errmsg << "Location '" << getID() << "' exit '" << new_exit.dir << "' flag '" << flagstr << 
																		" is not a recognized exit flag.";
            mudlog->writeLog(errmsg.str().c_str());
            return 0;
			}
			new_exit.eflags[i] = true;
		}
		_exits.push_back(new_exit);
   }

   // Get the Location Flags (if any)
   for (pugi::xml_node flag = entnode.child("flag"); flag; flag = flag.next_sibling("flag")) {
      try {
			pugi::xml_attribute attr = flag.attribute("name");
			if (attr == nullptr) {
				errmsg << "Location '" << getID() << "' flag node missing mandatory name field.";
				mudlog->writeLog(errmsg.str().c_str());
				return 0;				
			}
         setFlag(attr.value(), true);
      }
      catch (std::invalid_argument &e) {
         errmsg << "Location '" << getID() << "' flag error: " << e.what();
         mudlog->writeLog(errmsg.str().c_str());
         return 0;
      }
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
   if (Entity::setFlagInternal(flagname, newval))
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
   if (Entity::isFlagSetInternal(flagname, results))
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
 *
 *********************************************************************************************/

std::shared_ptr<Location> Location::getExit(const char *exitname) {
   if (exitname == NULL)
      return nullptr;

   for (unsigned int i=0; i<_exits.size(); i++) {
      if (_exits[i].dir.compare(exitname) == 0)
         return _exits[i].link_loc;
   }
   return nullptr;
}

std::shared_ptr<Location> Location::getExitAbbrev(const char *exitname) {
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

	for (unsigned int i=0; i<_exits.size(); i++) {
		if (_exits[i].dir.compare(0, dir.size(), dir) == 0)
			return _exits[i].link_loc;
	}
	return nullptr;
}
   //
/*********************************************************************************************
 * addLinks - Adds shared_ptr links between this object and others in the EntityDB. Polymorphic
 *
 *
 *********************************************************************************************/

void Location::addLinks(EntityDB &edb, std::shared_ptr<Entity> self) {
	std::stringstream msg;
	(void) self;

	// Go through the exits, creating links
	auto exit_it = _exits.begin();
	while (exit_it != _exits.end()) {
		std::shared_ptr<Entity> entptr = edb.getEntity(exit_it->link_id.c_str());
		std::shared_ptr<Location> locptr;

		if (entptr == nullptr) {
			msg << "Location '" << getID() << "' exit '" << exit_it->dir << "' doesn't appear to exist.";
			mudlog->writeLog(msg.str().c_str());
			exit_it = _exits.erase(exit_it);
		} else if ((locptr = std::dynamic_pointer_cast<Location>(entptr)) == nullptr) {
         msg << "Location '" << getID() << "' exit '" << exit_it->dir << "' doesn't appear to be a Location class.";
         mudlog->writeLog(msg.str().c_str());
         exit_it = _exits.erase(exit_it);
		} else {
			exit_it->link_loc = locptr;
			exit_it++;
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

		dir = _exits[i].dir;
		dir[0] = toupper(dir[0]);
		
		buf += "   ";
		buf += dir;
		buf += ": ";
		buf += _exits[i].link_loc->getTitle();
		buf += "\n";
		count++;
	}

	if (count == 0) 
		buf += "   None\n";
	return buf.c_str();
}

/*********************************************************************************************
 * getContained - given a name or altname, gets an entity contained within this static
 *
 *    Params:
 *             allow_abbrev - should matching allow for abbreviated versions?
 *
 *********************************************************************************************/

std::shared_ptr<Entity> Location::getContained(const char *name_alias, bool allow_abbrev) {
   std::string name = name_alias;

   std::shared_ptr<Entity> results = Entity::getContained(name_alias, allow_abbrev);
   if (results != nullptr)
      return results;

   // Still not found, check abbreviations
   auto eit = _contained.begin();
   for ( ; eit != _contained.end(); eit++) {
      if ((*eit)->hasAltName(name_alias, allow_abbrev))
         return *eit;
   }
   return nullptr;
}

/*********************************************************************************************
 * listContents - preps a string with a list of visible getables and organisms first
 *
 *
 *********************************************************************************************/

const char *Location::listContents(std::string &buf) {
	auto cit = _contained.begin();

	// Show getables first
	for (cit = _contained.begin(); cit != _contained.end(); cit++) {
      std::shared_ptr<Getable> gptr = std::dynamic_pointer_cast<Getable>(*cit);

      if (gptr == nullptr)
         continue;
	
		buf += gptr->getRoomDesc();
		buf += "\n";	
	}

   // Show organisms next
   for ( ; cit != _contained.end(); cit++) {
      std::shared_ptr<Organism> optr = std::dynamic_pointer_cast<Organism>(*cit);

      if (optr == nullptr)
         continue;

   }
	return buf.c_str();
}

