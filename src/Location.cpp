#include <iostream>
#include <sstream>
#include "Location.h"
#include "MUD.h"
#include "misc.h"
#include "global.h"

const char *lflag_list[] = {"outdoors", NULL};


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
	pugi::xml_node node = entnode.child("Desc");
   if (node == nullptr) {
      errmsg << "Location '" << getID() << "' missing mandatory Desc field.";
      mudlog->writeLog(errmsg.str().c_str());
      return 0;
   }
	setDesc(node.child_value());

   // Get the acttype - must be either hardcoded or script
   pugi::xml_attribute attr = entnode.attribute("Title");
   if (node == nullptr) {
      errmsg << "Location '" << getID() << "' missing mandatory Title field.";
      mudlog->writeLog(errmsg.str().c_str());
      return 0;
   }

	setTitle(attr.value());

   // Get the Action Flags (if any)
   for (pugi::xml_node flag = entnode.child("LocFlag"); flag; flag = flag.next_sibling("LocFlag")) {
      try {
         setFlag(flag.child_value(), true);
      }
      catch (std::invalid_argument &e) {
         errmsg << "Location '" << getID() << "' Flag error: " << e.what();
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



