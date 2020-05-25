#include <iostream>
#include <sstream>
#include <memory>
#include "Door.h"
#include "MUD.h"
#include "misc.h"
#include "global.h"
#include "Location.h"

const char *doorflag_list[] = {"hideclosedexit", NULL};


/*********************************************************************************************
 * Door (constructor) - 
 *
 *********************************************************************************************/
Door::Door(const char *id):
								Static(id)
{
	_roomdesc.assign(4, "");
	_typename = "Door";
}

// Copy constructor
Door::Door(const Door &copy_from):
								Static(copy_from)
{
}


Door::~Door() {

}

/*********************************************************************************************
 * saveData - Called by a child class to save Door-specific data into an XML document
 *
 *    Params:  entnode - This entity's node within the XML tree so attributes can be added to
 *                       it. This should be set up by a child class
 *
 *********************************************************************************************/

void Door::saveData(pugi::xml_node &entnode) const {

	// First, call the parent version
	Entity::saveData(entnode);

   // Add organism data (none yet)
   // pugi::xml_attribute idnode = entnode.append_attribute("id");
   // idnode.set_value(_id.c_str());

}

/*********************************************************************************************
 * loadData - Called by a child class to populate Door-specific data from an XML document
 *
 *    Params:  entnode - This entity's node within the XML tree so attributes can be drawn from
 *                       it
 *
 *    Returns: 1 for success, 0 for failure
 *
 *********************************************************************************************/

int Door::loadData(pugi::xml_node &entnode) {

	// First, call the parent function
	int results = 0;
	if ((results = Static::loadData(entnode)) != 1)
		return results;

	std::stringstream errmsg;


   // Get the second location for this door to start in
   pugi::xml_attribute attr = entnode.attribute("startloc2");
   if (attr == nullptr) {
      errmsg << "Door '" << getID() << "' missing mandatory startloc2 field.";
      mudlog->writeLog(errmsg.str().c_str());
      return 0;
   }
   setStartLoc2(attr.value());

   // Get the RoomDesc
   for (pugi::xml_node anode = entnode.child("roomdesc");	anode; anode = 
																					anode.next_sibling("roomdesc")) {

		// Get the state for thid desc
      pugi::xml_attribute attr = anode.attribute("state");
      if (attr == nullptr) {
         errmsg << "Door '" << getID() << "' roomdesc missing mandatory state field.";
         mudlog->writeLog(errmsg.str().c_str());
         return 0;
      }		

		std::string state = attr.value();
		lower(state);
	
		std::string rdstr(anode.child_value() == NULL ? "" : anode.child_value());

		if (state.compare("open") == 0)
			_roomdesc[Open] = rdstr;
		else if (state.compare("closed") == 0)
         _roomdesc[Closed] = rdstr;
      else if (state.compare("locked") == 0)
         _roomdesc[Locked] = rdstr;
      else if (state.compare("special") == 0)
         _roomdesc[Special] = rdstr;
      else {
         errmsg << "Door '" << getID() << "' roomdesc state not a valid state.";
         mudlog->writeLog(errmsg.str().c_str());
         return 0;
		} 
			
   }

   // Get the title that appears in most text in game
   attr = entnode.attribute("title");
   if (attr == nullptr) {
      errmsg << "Door '" << getID() << "' missing mandatory title field.";
      mudlog->writeLog(errmsg.str().c_str());
      return 0;
   }
   setTitle(attr.value());

	return 1;
}

/*********************************************************************************************
 * **** functions to set door attributes
 *
 *********************************************************************************************/

void Door::setStartLoc2(const char *newloc) {
   _startloc2 = newloc;
}



/*********************************************************************************************
 * setFlagInternal - given the flag string, first checks the parent for the flag, then checks
 *                   this class' flags
 *
 *
 *********************************************************************************************/

bool Door::setFlagInternal(const char *flagname, bool newval) {
   if (Static::setFlagInternal(flagname, newval))
      return true;

   std::string flagstr = flagname;
   lower(flagstr);

   size_t i=0;
   while ((doorflag_list[i] != NULL) && (flagstr.compare(doorflag_list[i]) != 0))
      i++;

   if (doorflag_list[i] == NULL)
      return false;

   _doorflags[i] = true;
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

bool Door::isFlagSetInternal(const char *flagname, bool &results) {
   if (Static::isFlagSetInternal(flagname, results))
      return true;

   std::string flagstr = flagname;
   lower(flagstr);

   size_t i=0;
   while ((doorflag_list[i] != NULL) && (flagstr.compare(doorflag_list[i]) != 0))
      i++;

   if (doorflag_list[i] == NULL)
      return false;

   results =_doorflags[i];
   return true;

}

/*********************************************************************************************
 * addLinks - Adds shared_ptr links between this object and others in the EntityDB. Polymorphic
 *
 *
 *********************************************************************************************/

void Door::addLinks(EntityDB &edb, std::shared_ptr<Entity> self) {
   std::stringstream msg;

	// Static should add the door to startloc1
	Static::addLinks(edb, self);

	// Check if the cur_loc is a Location--which it must be for doors
   if (std::dynamic_pointer_cast<Location>(getCurLoc()) == nullptr) {
      msg << "Object '" << getID() << "' startloc '" << getStartLoc() << "' must be assigned to a Location object.";
      mudlog->writeLog(msg.str().c_str());
      return;
   }


	// Now add this door to startloc2
   std::shared_ptr<Entity> entptr = edb.getEntity(_startloc2.c_str());

	if (entptr == nullptr) {
		msg << "Door '" << getID() << "' startloc2 '" << _startloc2 << "' doesn't appear to exist.";
      mudlog->writeLog(msg.str().c_str());
		return;
   } 

   // Static objects must be in a location object
   if (std::dynamic_pointer_cast<Location>(entptr) == nullptr) {
      msg << "Object '" << getID() << "' startloc2 '" << _startloc2 << "' must be assigned to a Location object.";
      mudlog->writeLog(msg.str().c_str());
      return;
   }

	// Add this door to the second room 
   _cur_loc2 = entptr;

   entptr->addEntity(self);


}

/*********************************************************************************************
 * getOppositeLoc - Given the parameter location, provides the other location that this door is
 *                  contained within.
 *
 *********************************************************************************************/

std::shared_ptr<Entity> Door::getOppositeLoc(std::shared_ptr<Entity> cur_loc) {
	std::shared_ptr<Location> locptr = std::dynamic_pointer_cast<Location>(cur_loc);
	return getOppositeLoc(&(*locptr));
}

std::shared_ptr<Entity> Door::getOppositeLoc(Location *cur_loc) {
	std::shared_ptr<Entity> loc1(getCurLoc()), loc2(getCurLoc2());

	
   if (*loc1 == cur_loc)
      return loc2;
   else if (*loc2 == cur_loc)
      return loc1;
   else
   {
      std::string msg("Door::getOppositeLoc called on door '");
      msg += getID();
      msg += "' with parameter not one of the door's location.";
      mudlog->writeLog(msg.c_str());
      return nullptr;
   }
}

