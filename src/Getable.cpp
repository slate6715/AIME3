#include <iostream>
#include <sstream>
#include <memory>
#include "Getable.h"
#include "MUD.h"
#include "misc.h"
#include "global.h"

const char *gflag_list[] = {"noget", "nodrop", NULL};


/*********************************************************************************************
 * Getable (constructor) - 
 *
 *********************************************************************************************/
Getable::Getable(const char *id):
								Static(id)
{
	_typename = "Getable";

}

// Copy constructor
Getable::Getable(const Getable &copy_from):
								Static(copy_from)
{

}


Getable::~Getable() {

}

/*********************************************************************************************
 * saveData - Called by a child class to save Getable-specific data into an XML document
 *
 *    Params:  entnode - This entity's node within the XML tree so attributes can be added to
 *                       it. This should be set up by a child class
 *
 *********************************************************************************************/

void Getable::saveData(pugi::xml_node &entnode) const {

	// First, call the parent version
	Static::saveData(entnode);

   // Add organism data (none yet)
   // pugi::xml_attribute idnode = entnode.append_attribute("id");
   // idnode.set_value(_id.c_str());

}

/*********************************************************************************************
 * loadData - Called by a child class to populate Getable-specific data from an XML document
 *
 *    Params:  entnode - This entity's node within the XML tree so attributes can be drawn from
 *                       it
 *
 *    Returns: 1 for success, 0 for failure
 *
 *********************************************************************************************/

int Getable::loadData(pugi::xml_node &entnode) {

	// First, call the parent function
	int results = 0;
	if ((results = Static::loadData(entnode)) != 1)
		return results;

	std::stringstream errmsg;

   // Get the Altnames (if any)
   for (pugi::xml_node anode = entnode.child("roomdesc");	anode; anode = 
																					anode.next_sibling("roomdesc")) {
		pushRoomDesc(anode.child_value());
   }

	if (_roomdesc.size() == 0) {
      errmsg << "Getable '" << getID() << "' must have at least one roomdesc defined.";
      mudlog->writeLog(errmsg.str().c_str());
      return 0;
	}

   // Get the acttype - must be either hardcoded or script
   pugi::xml_attribute attr = entnode.attribute("title");
   if (attr == nullptr) {
      errmsg << "Getable '" << getID() << "' missing mandatory title field.";
      mudlog->writeLog(errmsg.str().c_str());
      return 0;
   }
	setTitle(attr.value());

	return 1;
}


/*********************************************************************************************
 * setFlagInternal - given the flag string, first checks the parent for the flag, then checks
 *                   this class' flags
 *
 *
 *********************************************************************************************/

bool Getable::setFlagInternal(const char *flagname, bool newval) {
   if (Static::setFlagInternal(flagname, newval))
      return true;

   std::string flagstr = flagname;
   lower(flagstr);

   size_t i=0;
   while ((gflag_list[i] != NULL) && (flagstr.compare(gflag_list[i]) != 0))
      i++;

   if (gflag_list[i] == NULL)
      return false;

   _getflags[i] = true;
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

bool Getable::isFlagSetInternal(const char *flagname, bool &results) {
   if (Static::isFlagSetInternal(flagname, results))
      return true;

   std::string flagstr = flagname;
   lower(flagstr);

   size_t i=0;
   while ((gflag_list[i] != NULL) && (flagstr.compare(gflag_list[i]) != 0))
      i++;

   if (gflag_list[i] == NULL)
      return false;

   results =_getflags[i];
   return true;

}

/*********************************************************************************************
 * pushRoomDesc, popRoomDesc, getRoomDesc - manages the roomDesc stack that a user sees when
 *				they look in the room. Only the top on the stack is seen. When an item is gotten, 
 *				typically the stack gets popped.. NOTE: will not allow the last one to be popped.
 *
 *
 *********************************************************************************************/

   // Manages the roomdesc--the description one sees when they look in the room
void Getable::pushRoomDesc(const char *new_desc){
	_roomdesc.push(std::string(new_desc));
}

void Getable::popRoomDesc() {
	// Don't let them pop the last roomdesc
	if (_roomdesc.size() == 1) {
		std::stringstream msg;
		msg << "Attempt to remove the last RoomDesc from the stack, Getable '" << 
						getID() << "'";
		mudlog->writeLog(msg.str().c_str());
		return;
	}
	
	_roomdesc.pop();
}

const char *Getable::getRoomDesc() {
	return _roomdesc.top().c_str();
}

/*********************************************************************************************
 * **** functions to set location attributes
 *
 *********************************************************************************************/

void Getable::setTitle(const char *newtitle) {
   _title = newtitle;
}

/*********************************************************************************************
 * listContents - preps a string with a list of visible objects in a getable
 *
 *
 *********************************************************************************************/

const char *Getable::listContents(std::string &buf, const Entity *exclude) const {
   auto cit = _contained.begin();

   // Show getables first
   for ( ; cit != _contained.end(); cit++) {
      std::shared_ptr<Getable> gptr = std::dynamic_pointer_cast<Getable>(*cit);

      if ((gptr == nullptr) || (*gptr == exclude))
         continue;

      buf += gptr->getRoomDesc();
      buf += "\n";
   }

   return buf.c_str();
}


