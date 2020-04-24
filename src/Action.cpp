#include "Action.h"


/*********************************************************************************************
 * Action (constructor) - Called by a child class to initialize any Action elements
 *
 *********************************************************************************************/
Action::Action(const char *id):
								Entity(id) 
{


}

// Called by child class
Action::Action(const Action &copy_from):
								Entity(copy_from)
{

}


Action::~Action() {

}

/*********************************************************************************************
 * saveData - Called by a child class to save Action-specific data into an XML document
 *
 *    Params:  entnode - This entity's node within the XML tree so attributes can be added to
 *                       it. This should be set up by a child class
 *             log - to log any errors
 *
 *********************************************************************************************/

void Action::saveData(pugi::xml_node &entnode) const {

	// First, call the parent version
	Entity::saveData(entnode);

   // Add organism data (none yet)
   // pugi::xml_attribute idnode = entnode.append_attribute("id");
   // idnode.set_value(_id.c_str());

}

/*********************************************************************************************
 * loadData - Called by a child class to populate Action-specific data from an XML document
 *
 *    Params:  entnode - This entity's node within the XML tree so attributes can be drawn from
 *                       it
 *             log - to log any errors
 *
 *    Returns: 1 for success, 0 for failure
 *
 *********************************************************************************************/

int Action::loadData(LogMgr &log, pugi::xml_node &entnode) {

	// First, call the parent function
	int results = 0;
	if ((results = Entity::loadData(log, entnode)) != 1)
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


