#include <iostream>
#include <sstream>
#include "Script.h"

Script::Script(const char *id):
								Entity(id)
{
	_typename = "Script";
}

Script::Script(const Script &copy_from):
										Entity(copy_from)
{

}

Script::~Script() {

}

/*********************************************************************************************
 * saveData - Called by a child class to save Script-specific data into an XML document
 *
 *    Params:  entnode - This entity's node within the XML tree so attributes can be added to
 *                       it. This should be set up by a child class
 *             log - to log any errors
 *
 *********************************************************************************************/

void Script::saveData(pugi::xml_node &entnode) const {

   // First, call the parent version
   Entity::saveData(entnode);

   // Add organism data (none yet)
   // pugi::xml_attribute idnode = entnode.append_attribute("id");
   // idnode.set_value(_id.c_str());

}

/*********************************************************************************************
 * loadData - Called by a child class to populate Script-specific data from an XML document
 *
 *    Params:  entnode - This entity's node within the XML tree so attributes can be drawn from
 *                       it
 *             log - to log any errors
 *
 *    Returns: 1 for success, 0 for failure
 *
 *********************************************************************************************/

int Script::loadData(pugi::xml_node &entnode) {
   // First, call the parent function
   int results = 0;
   if ((results = Entity::loadData(entnode)) != 1)
      return results;

   std::stringstream errmsg;

	return 1;
}
