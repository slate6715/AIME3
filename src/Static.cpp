#include <iostream>
#include <sstream>
#include <memory>
#include "Static.h"
#include "MUD.h"
#include "misc.h"
#include "global.h"

const char *sflag_list[] = {"outdoors", "bright", "death", NULL};


/*********************************************************************************************
 * Static (constructor) - 
 *
 *********************************************************************************************/
Static::Static(const char *id):
								Entity(id)
{


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
			pugi::xml_attribute attr = flag.attribute("name");
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

 
	return 1;
}

/*********************************************************************************************
 * **** functions to set staticn attributes
 *
 *********************************************************************************************/

void Static::setDesc(const char *newdesc) {
	_desc = newdesc;
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


