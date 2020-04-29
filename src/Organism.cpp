#include "Organism.h"
#include "global.h"

/*********************************************************************************************
 * Organism (constructor) - Called by a child class to initialize any Organism elements
 *
 *********************************************************************************************/
Organism::Organism(const char *id):
								Entity(id) 
{


}

// Called by child class
Organism::Organism(const Organism &copy_from):
								Entity(copy_from)
{

}


Organism::~Organism() {

}

/*********************************************************************************************
 * saveData - Called by a child class to save Organism-specific data into an XML document
 *
 *    Params:  entnode - This entity's node within the XML tree so attributes can be added to
 *                       it. This should be set up by a child class
 *             log - to log any errors
 *
 *********************************************************************************************/

void Organism::saveData(pugi::xml_node &entnode) const {

	// First, call the parent version
	Entity::saveData(entnode);

   // Add organism data (none yet)
   // pugi::xml_attribute idnode = entnode.append_attribute("id");
   // idnode.set_value(_id.c_str());

}

/*********************************************************************************************
 * loadData - Called by a child class to populate Organism-specific data from an XML document
 *
 *    Params:  entnode - This entity's node within the XML tree so attributes can be drawn from
 *                       it
 *
 *    Returns: 1 for success, 0 for failure
 *
 *********************************************************************************************/

int Organism::loadData(pugi::xml_node &entnode) {

	// First, call the parent function
	int results = 0;
	if ((results = Entity::loadData(entnode)) != 1)
		return results;

	// Now populate organism data (none yet)
   // pugi::xml_attribute attr = entnode.find_attribute("id");
   // if (attr == nullptr) {
   //    log.writeLog("Entity save file missing mandatory 'id' field.", 2);
   //    return 0;
   // }
   // _id = attr.value();

	return 1;
}

/*********************************************************************************************
 * setFlagInternal - given the flag string, first checks the parent for the flag, then checks
 *                   this class' flags
 *
 *
 *********************************************************************************************/

bool Organism::setFlagInternal(const char *flagname, bool newval) {
   if (Entity::setFlagInternal(flagname, newval))
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

bool Organism::isFlagSetInternal(const char *flagname, bool &results) {
   if (Entity::isFlagSetInternal(flagname, results))
      return true;

	return false;
}

