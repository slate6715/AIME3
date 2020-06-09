#include <iostream>
#include <sstream>
#include <memory>
#include "Static.h"
#include "MUD.h"
#include "misc.h"
#include "global.h"
#include "Getable.h"
#include "Door.h"

const char *sflag_list[] = {"container", "lockable", "closeable", "lightable", "magiclit", NULL};


/*********************************************************************************************
 * Static (constructor) - 
 *
 *********************************************************************************************/
Static::Static(const char *id):
								Entity(id)
{
	_typename = "Static";

}

// Copy constructor
Static::Static(const Static &copy_from):
								Entity(copy_from)
{

}


Static::~Static() {

}

/*********************************************************************************************
 * saveData - Called by a child class to save Static-specific data into an XML document
 *
 *    Params:  entnode - This entity's node within the XML tree so attributes can be added to
 *                       it. This should be set up by a child class
 *
 *********************************************************************************************/

void Static::saveData(pugi::xml_node &entnode) const {

	// First, call the parent version
	Entity::saveData(entnode);

   // Add organism data (none yet)
   // pugi::xml_attribute idnode = entnode.append_attribute("id");
   // idnode.set_value(_id.c_str());

}

/*********************************************************************************************
 * loadData - Called by a child class to populate Static-specific data from an XML document
 *
 *    Params:  entnode - This entity's node within the XML tree so attributes can be drawn from
 *                       it
 *
 *    Returns: 1 for success, 0 for failure
 *
 *********************************************************************************************/

int Static::loadData(pugi::xml_node &entnode) {

	// First, call the parent function
	int results = 0;
	if ((results = Entity::loadData(entnode)) != 1)
		return results;

	std::stringstream errmsg;

   // Get the acttype - must be either hardcoded or script
	pugi::xml_node node = entnode.child("desc");
   if (node == nullptr) {
      errmsg << "Static '" << getID() << "' missing mandatory desc field.";
      mudlog->writeLog(errmsg.str().c_str());
      return 0;
   }
	setDesc(node.child_value());

   // Get the acttype - must be either hardcoded or script
   pugi::xml_attribute attr = entnode.attribute("startloc");
   if (attr == nullptr) {
      errmsg << "Static '" << getID() << "' missing mandatory startloc field.";
      mudlog->writeLog(errmsg.str().c_str());
      return 0;
   }
   setStartLoc(attr.value());

   // Get the Altnames (if any)
   for (pugi::xml_node anode = entnode.child("altname");	anode; anode = 
																					anode.next_sibling("altname")) {

		// Get the direction name such as "east"
      pugi::xml_attribute attr = anode.attribute("name");
      if (attr == nullptr) {
         errmsg << "Static '" << getID() << "' Altname node missing mandatory name field.";
         mudlog->writeLog(errmsg.str().c_str());
         return 0;
      }		
		_altnames.push_back(attr.value());
   }

   // Get the Static Flags (if any)
   for (pugi::xml_node flag = entnode.child("flag"); flag; flag = flag.next_sibling("flag")) {
      try {
			attr = flag.attribute("name");
			if (attr == nullptr) {
				errmsg << "Static '" << getID() << "' flag node missing mandatory name field.";
				mudlog->writeLog(errmsg.str().c_str());
				return 0;				
			}
         setFlag(attr.value(), true);
      }
      catch (std::invalid_argument &e) {
         errmsg << "Static '" << getID() << "' flag error: " << e.what();
         mudlog->writeLog(errmsg.str().c_str());
         return 0;
      }
   }

   // Get the state (open, closed, etc) this container or door starts in
   if ((attr = entnode.attribute("doorstate")) != nullptr) {
		if (!setDoorState(attr.value())) {
			errmsg << "Object '" << getID() << "' doorstate '" << attr.value() << "' not a valid state.";
			mudlog->writeLog(errmsg.str().c_str());
			return 0;
		}

   }

   // Get the keys that unlock this container (assuming it is lockable) 
   for (pugi::xml_node anode = entnode.child("key"); anode; anode =
                                                               anode.next_sibling("key")) {

      // Get the id of the key
      pugi::xml_attribute attr = anode.attribute("id");
      if (attr == nullptr) {
         errmsg << "Object '" << getID() << "' key node missing mandatory id field.";
         mudlog->writeLog(errmsg.str().c_str());
         return 0;
      }

      _keys.push_back(attr.value());

   }
 
 
	return 1;
}

/*********************************************************************************************
 * **** functions to set staticn attributes
 *
 *********************************************************************************************/

void Static::setDesc(const char *newdesc) {
	_desc = newdesc;
}

void Static::setStartLoc(const char *newloc) {
   _startloc = newloc;
}

void Static::addAltName(const char *names) {
   _altnames.push_back(names);
}


/*********************************************************************************************
 * setFlagInternal - given the flag string, first checks the parent for the flag, then checks
 *                   this class' flags
 *
 *
 *********************************************************************************************/

bool Static::setFlagInternal(const char *flagname, bool newval) {
   if (Entity::setFlagInternal(flagname, newval))
      return true;

   std::string flagstr = flagname;
   lower(flagstr);

   size_t i=0;
   while ((sflag_list[i] != NULL) && (flagstr.compare(sflag_list[i]) != 0))
      i++;

   if (sflag_list[i] == NULL)
      return false;

   _staticflags[i] = true;
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

bool Static::isFlagSetInternal(const char *flagname, bool &results) {
   if (Entity::isFlagSetInternal(flagname, results))
      return true;

   std::string flagstr = flagname;
   lower(flagstr);

   size_t i=0;
   while ((sflag_list[i] != NULL) && (flagstr.compare(sflag_list[i]) != 0))
      i++;

   if (sflag_list[i] == NULL)
      return false;

   results =_staticflags[i];
   return true;

}

/*********************************************************************************************
 * hasAltName - looks through the list of alt names to see if one matches the parameter
 *
 *		Params:	str - the string we're trying to match
 *					allow_abbrev - should matching allow for abbreviated versions?
 *
 *********************************************************************************************/

bool Static::hasAltName(const char *str, bool allow_abbrev) {
	std::string buf = str;

	for (unsigned int i=0; i<_altnames.size(); i++) {
		if (allow_abbrev) {
			if (buf.compare(0, buf.size(), _altnames[i]) == 0)
				return true;
		}
		else {
			if (buf.compare(str) == 0)
				return true;
		}
	}
	return false;

}

/*********************************************************************************************
 * addLinks - Adds shared_ptr links between this object and others in the EntityDB. Polymorphic
 *
 *
 *********************************************************************************************/

void Static::addLinks(EntityDB &edb, std::shared_ptr<Entity> self) {
   std::stringstream msg;

	// Place this at its start location
   std::shared_ptr<Entity> entptr = edb.getEntity(_startloc.c_str());

	if (entptr == nullptr) {
		msg << "Object '" << getID() << "' startloc '" << _startloc << "' doesn't appear to exist.";
      mudlog->writeLog(msg.str().c_str());
		return;
   } 

	moveEntity(entptr, self);

}

/*********************************************************************************************
 * getContained - given a name or altname, gets an entity contained within this static
 *
 *    Params:   
 *             allow_abbrev - should matching allow for abbreviated versions?
 *
 *********************************************************************************************/

std::shared_ptr<Entity> Static::getContained(const char *name_alias, bool allow_abbrev) {
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
 * listContents - preps a string with a list of visible items in this static's container
 *
 *
 *********************************************************************************************/

const char *Static::listContents(std::string &buf) const {
   auto cit = _contained.begin();

   // Show getables first
   for (cit = _contained.begin(); cit != _contained.end(); cit++) {
      std::shared_ptr<Getable> gptr = std::dynamic_pointer_cast<Getable>(*cit);

      if (gptr == nullptr)
         continue;

      buf += gptr->getTitle();
      buf += "\n";
   }

   return buf.c_str();
}

/*********************************************************************************************
 * setDoorState - changes the door state given a string as the parameter
 *
 *    Params: new_state - a string of open, closed, locked, or special
 *
 *    Returns: true if successful, false if the string was not recognized
 *
 *********************************************************************************************/

bool Static::setDoorState(const char *new_state) {
   std::string state = new_state;
   lower(state);

   if (state.compare("open") == 0) {
      setDoorState(Open);
   } else if (state.compare("closed") == 0) {
      setDoorState(Closed);
   } else if (state.compare("locked") == 0) {
      setDoorState(Locked);
   } else if (state.compare("special") == 0) {
      setDoorState(Special);
   } else {
      return false;
   }
   return true;

}

/*********************************************************************************************
 * open - does some checks and, if allowed, opens the container so it can be viewed and items
 *        exchanged
 *
 *    Params: errmsg - error text is stored here if the function fails its checks
 *
 *    Returns: true if successful, false if an error happened
 *
 *********************************************************************************************/

bool Static::open(std::string &errmsg) {
	
	// First, it must be a container or a door
	if (!isStaticFlagSet(Container) && (dynamic_cast<Door *>(this) == NULL)) {
		errmsg = "You can't open that.\n";
		return false;
	}

	// It must be unlocked
	if (getDoorState() == Locked) {
		errmsg = "It's locked.\n";
		return false;
	}

	if (getDoorState() == Open) {
		errmsg = "It is already open.\n";
		return false;
	}

	setDoorState(Open);
	return true;
}

/*********************************************************************************************
 * close - does some checks and, if allowed, closes the container
 *
 *    Params: errmsg - error text is stored here if the function fails its checks
 *
 *    Returns: true if successful, false if an error happened
 *
 *********************************************************************************************/

bool Static::close(std::string &errmsg) {

   // First, it must be a container or a door
   if (!isStaticFlagSet(Container) && (dynamic_cast<Door *>(this) == NULL)) {
      errmsg = "You can't close that.\n";
      return false;
   }

	// It must be closeable
	if (!isStaticFlagSet(Closeable)) {
		errmsg = "That can't be closed.\n";
		return false;
	}

   // It must be open
   if (getDoorState() != Open) {
      errmsg = "It is already closed.\n";
      return false;
   }

   setDoorState(Closed);
   return true;
}

