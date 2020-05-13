#include <sstream>
#include "NPC.h"
#include "global.h"
#include "misc.h"


const char *NPC_attriblist[] = {"aggression", "speed", NULL};

/*********************************************************************************************
 * NPC (constructor) - Called by a child class to initialize any NPC elements
 *
 *********************************************************************************************/
NPC::NPC(const char *id):
								Organism(id) 
{
	_typename = "NPC";
}

// Called by child class
NPC::NPC(const NPC &copy_from):
								Organism(copy_from)
{

}


NPC::~NPC() {

}

/*********************************************************************************************
 * saveData - Called by a child class to save NPC-specific data into an XML document
 *
 *    Params:  entnode - This entity's node within the XML tree so attributes can be added to
 *                       it. This should be set up by a child class
 *             log - to log any errors
 *
 *********************************************************************************************/

void NPC::saveData(pugi::xml_node &entnode) const {

	// First, call the parent version
	Organism::saveData(entnode);

   // Add organism data (none yet)
   // pugi::xml_attribute idnode = entnode.append_attribute("id");
   // idnode.set_value(_id.c_str());

}

/*********************************************************************************************
 * loadData - Called by a child class to populate NPC-specific data from an XML document
 *
 *    Params:  entnode - This entity's node within the XML tree so attributes can be drawn from
 *                       it
 *
 *    Returns: 1 for success, 0 for failure
 *
 *********************************************************************************************/

int NPC::loadData(pugi::xml_node &entnode) {

	std::stringstream errmsg;

	// First, call the parent function
	int results = 0;
	if ((results = Organism::loadData(entnode)) != 1)
		return results;

   // Get the acttype - must be either hardcoded or script
   pugi::xml_attribute attr = entnode.attribute("startloc");
   if (attr == nullptr) {
      errmsg << "NPC '" << getID() << "' missing mandatory startloc field.";
      mudlog->writeLog(errmsg.str().c_str());
      return 0;
   }
   setStartLoc(attr.value());

	return 1;
}

/*********************************************************************************************
 * setFlagInternal - given the flag string, first checks the parent for the flag, then checks
 *                   this class' flags
 *
 *
 *********************************************************************************************/

bool NPC::setFlagInternal(const char *flagname, bool newval) {
   if (Organism::setFlagInternal(flagname, newval))
      return true;

   // Here we would look for player flags
	return false;
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

bool NPC::isFlagSetInternal(const char *flagname, bool &results) {
   if (Organism::isFlagSetInternal(flagname, results))
      return true;

	return false;
}

/*********************************************************************************************
 * **** functions to set NPC attributes
 *
 *********************************************************************************************/

void NPC::setStartLoc(const char *newloc) {
   _startloc = newloc;
}

/*********************************************************************************************
 * addLinks - Adds shared_ptr links between this object and others in the EntityDB. Polymorphic
 *
 *
 *********************************************************************************************/

void NPC::addLinks(EntityDB &edb, std::shared_ptr<Entity> self) {
   std::stringstream msg;

   // Place this at its start location
   std::shared_ptr<Entity> entptr = edb.getEntity(_startloc.c_str());

   if (entptr == nullptr) {
      msg << "NPC '" << getID() << "' startloc '" << _startloc << "' doesn't appear to exist.";
      mudlog->writeLog(msg.str().c_str());
      return;
   }
   moveEntity(entptr, self);

}

