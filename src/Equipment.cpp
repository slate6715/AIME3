#include <iostream>
#include <sstream>
#include <memory>
#include "Equipment.h"
#include "MUD.h"
#include "misc.h"
#include "global.h"

const char *equipflag_list[] = {"twohanded", "antimagic", "outerlayer", NULL};


/*********************************************************************************************
 * Equipment (constructor) - 
 *
 *********************************************************************************************/
Equipment::Equipment(const char *id):
								Getable(id)
{
	_typename = "Equipment";

}

// Copy constructor
Equipment::Equipment(const Equipment &copy_from):
								Getable(copy_from)
{

}


Equipment::~Equipment() {

}

/*********************************************************************************************
 * saveData - Called by a child class to save Equipment-specific data into an XML document
 *
 *    Params:  entnode - This entity's node within the XML tree so attributes can be added to
 *                       it. This should be set up by a child class
 *
 *********************************************************************************************/

void Equipment::saveData(pugi::xml_node &entnode) const {

	// First, call the parent version
	Getable::saveData(entnode);

   // Add organism data (none yet)
   // pugi::xml_attribute idnode = entnode.append_attribute("id");
   // idnode.set_value(_id.c_str());

}

/*********************************************************************************************
 * loadData - Called by a child class to populate Equipment-specific data from an XML document
 *
 *    Params:  entnode - This entity's node within the XML tree so attributes can be drawn from
 *                       it
 *
 *    Returns: 1 for success, 0 for failure
 *
 *********************************************************************************************/

int Equipment::loadData(pugi::xml_node &entnode) {

	std::stringstream errmsg;

	// First, call the parent function
	int results = 0;
	if ((results = Getable::loadData(entnode)) != 1)
		return results;

   // Get the acttype - must be either hardcoded or script
   pugi::xml_attribute attr = entnode.attribute("startequipped");
   if (attr == nullptr) {
      errmsg << "Equipment '" << getID() << "' missing mandatory startequipped field.";
      mudlog->writeLog(errmsg.str().c_str());
      return 0;
   }

	std::string se_str = attr.value();
	if (se_str.compare("true") == 0) {
		_start_equipped = true;
	} else if (se_str.compare("false") == 0) {
		_start_equipped = false;
	} else {
      errmsg << "Equipment '" << getID() << 
							"' startequipped field has an invalid value. Should be either true or false.";
      mudlog->writeLog(errmsg.str().c_str());
      return 0;
	}

   for (pugi::xml_node equip = entnode.child("equipped"); equip; equip =
                                                               equip.next_sibling("equipped")) {
      try {
         pugi::xml_attribute attr = equip.attribute("bpname");
         if (attr == nullptr) {
            errmsg << getTypeName() << " '" << getID() << "' equipped node missing mandatory bpname field.";
            mudlog->writeLog(errmsg.str().c_str());
            return 0;
         }
         std::string bpname = attr.value();

         attr = equip.attribute("bpgroup");
         if (attr == nullptr) {
            errmsg << getTypeName() << " '" << getID() << "' equipped node missing mandatory bpgroup field.";
            mudlog->writeLog(errmsg.str().c_str());
            return 0;
         }

			_equip_list.push_back(std::pair<std::string, std::string>(bpname, attr.value()));
      }
      catch (std::invalid_argument &e) {
         errmsg << getTypeName() << " '" << getID() << "' equipment node error: " << e.what();
         mudlog->writeLog(errmsg.str().c_str());
         return 0;
      }
   }


	return 1;
}


/*********************************************************************************************
 * setFlagInternal - given the flag string, first checks the parent for the flag, then checks
 *                   this class' flags
 *
 *
 *********************************************************************************************/

bool Equipment::setFlagInternal(const char *flagname, bool newval) {
   if (Getable::setFlagInternal(flagname, newval))
      return true;

   std::string flagstr = flagname;
   lower(flagstr);

   size_t i=0;
   while ((equipflag_list[i] != NULL) && (flagstr.compare(equipflag_list[i]) != 0))
      i++;

   if (equipflag_list[i] == NULL)
      return false;

   _equipflags[i] = true;
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

bool Equipment::isFlagSetInternal(const char *flagname, bool &results) {
   if (Getable::isFlagSetInternal(flagname, results))
      return true;

   std::string flagstr = flagname;
   lower(flagstr);

   size_t i=0;
   while ((equipflag_list[i] != NULL) && (flagstr.compare(equipflag_list[i]) != 0))
      i++;

   if (equipflag_list[i] == NULL)
      return false;

   results =_equipflags[i];
   return true;

}

/*********************************************************************************************
 * addLinks - Adds shared_ptr links between this object and others in the EntityDB. Polymorphic
 *
 *
 *********************************************************************************************/

void Equipment::addLinks(EntityDB &edb, std::shared_ptr<Physical> self) {
   std::stringstream errmsg;

	// Will add the equipment to its start location
	Getable::addLinks(edb, self);
	
	// Equip the equipment if it is supposed to start being worn
	if (_start_equipped) {
		std::shared_ptr<Organism> optr = std::dynamic_pointer_cast<Organism>(getCurLoc());

		if (optr == nullptr) {
			errmsg << "Equipment '" << getID() << "' startequipped set to true but startloc '" << getStartLoc() << 
											" not an Organism type.";
			mudlog->writeLog(errmsg.str().c_str());
			return;
		}

		std::string buf;
		if (optr->equip(self, buf) <= 0) {
         errmsg << "Equipment '" << getID() << "' error equipping at start organism '" << getStartLoc() << "'"; 
         mudlog->writeLog(errmsg.str().c_str());
         return;

		}
	}
}


