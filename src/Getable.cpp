#include <iostream>
#include <sstream>
#include <memory>
#include "Getable.h"
#include "MUD.h"
#include "misc.h"
#include "global.h"

const char *gflag_list[] = {"noget", "nodrop", "food", "rope", "luckfast", "thiefonly",  
								"blockmagic", "fastheal", NULL};


/*********************************************************************************************
 * Getable (constructor) - 
 *
 *********************************************************************************************/
Getable::Getable(const char *id):
								Static(id)
{
	_typename = "Getable";

   // Make sure we have at least the bare minimum roomdesc
   _roomdesc.assign(Custom, std::pair<std::string, std::string>("", ""));

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
	   // Get the state (open, closed, etc) this container or door starts in
		pugi::xml_attribute attr;
		if ((attr = anode.attribute("state")) == nullptr) {
			errmsg << "Getable Object '" << getID() << "' roomdesc missing mandatory state attribute.";
			mudlog->writeLog(errmsg.str().c_str());
			return 0;
		}
		std::string label = attr.value();

		lower(label);

		if (label.compare("pristine") == 0) {
			setRoomDesc(Pristine, anode.child_value());
		} else if (label.compare("dropped") == 0) {
         setRoomDesc(Dropped, anode.child_value());
      } else if (label.compare("lit") == 0) {
         setRoomDesc(Lit, anode.child_value());
      } else if (label.compare("depleted") == 0) {
         setRoomDesc(Depleted, anode.child_value());
      } else if (label.compare("custom") == 0) {
			if ((attr = anode.attribute("customname")) == nullptr) {
	         errmsg << "Getable Object '" << getID() << "' custom roomdesc missing mandatory customname attribute.";
		      mudlog->writeLog(errmsg.str().c_str());
			   return 0;
			}
         setRoomDesc(Custom, anode.child_value(), attr.value());
		} else {
         errmsg << "Getable Object '" << getID() << "' roomdesc state '" << label << "' not a recognized state.";
         mudlog->writeLog(errmsg.str().c_str());
         return 0;
		}
   }

	if (_roomdesc.size() == 0) {
      errmsg << "Getable '" << getID() << "' must have at least one roomdesc defined.";
      mudlog->writeLog(errmsg.str().c_str());
      return 0;
	}

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
 * setRoomDesc - changes the text on a RoomDesc for that particular state. If new_state is 
 *				set to Custom, then customname must be populated
 *
 *		Throws:	invalid_argument if there's an issue with customname
 *
 *********************************************************************************************/

void Getable::setRoomDesc(descstates new_state, const char *new_desc, const char *customname) {
	if (new_state == Custom) {
		if (customname == NULL)
			throw std::invalid_argument("Attempt to set Custom roomDesc but no customname is set");

		std::string cname(customname);

		// First see if it already exists
		for (unsigned int i=Custom; i<_roomdesc.size(); i++) {
			if (cname.compare(_roomdesc[i].second.c_str()) == 0) {
				_roomdesc[i].first = new_desc;
				return;
			}			
		}

		// It wasn't found, add it
		_roomdesc.push_back(std::pair<std::string, std::string>(new_desc, customname));
		return;
	}

	_roomdesc[new_state].first = new_desc;
}

/*********************************************************************************************
 * changeRoomDesc - changes the RoomDesc state of this Getable. If the newstate is set to Custom, 
 *                  then customname must be populated
 *
 *    Throws:  invalid_argument if there's an issue with customname
 *
 *********************************************************************************************/

void Getable::changeRoomDesc(descstates new_state, const char *customname) {
   if (new_state == Custom) {
      if (customname == NULL)
         throw std::invalid_argument("Attempt to change to a Custom roomDesc but no customname is set");

      std::string cname(customname);

      // First see if it already exists
      for (unsigned int i=Custom; i<_roomdesc.size(); i++) {
         if (cname.compare(_roomdesc[i].second.c_str()) == 0) {
				_dstate = i;
            return;
         }
      }

		throw std::invalid_argument("Attempt to change to a Custom roomDesc but the customname doesn't exist");
   }

   // A little more logic--if no review is set in Dropped
   if ((new_state == Lit) && (_roomdesc[new_state].first.size() == 0))
      new_state = Dropped;

   if ((new_state == Dropped) && (_roomdesc[new_state].first.size() == 0))
      new_state = Pristine;

	_dstate = new_state; 
}

/*********************************************************************************************
 * getRoomDesc - gets the currently active roomdesc
 *
 *
 *********************************************************************************************/

const char *Getable::getRoomDesc() {
	return _roomdesc[_dstate].first.c_str();
}

/*********************************************************************************************
 * **** functions to set location attributes
 *
 *********************************************************************************************/


/*********************************************************************************************
 * listContents - preps a string with a list of visible objects in a getable
 *
 *
 *********************************************************************************************/

const char *Getable::listContents(std::string &buf, const Physical *exclude) const {
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


