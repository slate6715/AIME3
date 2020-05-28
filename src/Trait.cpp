#include <iostream>
#include <sstream>
#include <memory>
#include "Trait.h"
#include "MUD.h"
#include "misc.h"
#include "global.h"


/*********************************************************************************************
 * Trait (constructor) - 
 *
 *********************************************************************************************/
Trait::Trait(const char *id):
								Entity(id)
{
	_typename = "Trait";

}

// Copy constructor
Trait::Trait(const Trait &copy_from):
								Entity(copy_from)
{

}


Trait::~Trait() {

}

/*********************************************************************************************
 * saveData - Called by a child class to save Trait-specific data into an XML document
 *
 *    Params:  entnode - This entity's node within the XML tree so attributes can be added to
 *                       it. This should be set up by a child class
 *
 *********************************************************************************************/

void Trait::saveData(pugi::xml_node &entnode) const {

	// First, call the parent version
	Entity::saveData(entnode);

   // Add organism data (none yet)
   // pugi::xml_attribute idnode = entnode.append_attribute("id");
   // idnode.set_value(_id.c_str());

}

/*********************************************************************************************
 * loadData - Called by a child class to populate Trait-specific data from an XML document
 *
 *    Params:  entnode - This entity's node within the XML tree so attributes can be drawn from
 *                       it
 *
 *    Returns: 1 for success, 0 for failure
 *
 *********************************************************************************************/

int Trait::loadData(pugi::xml_node &entnode) {

	// First, call the parent function
	int results = 0;
	if ((results = Entity::loadData(entnode)) != 1)
		return results;

	std::stringstream errmsg;

	for (pugi::xml_node maskattr = entnode.child("maskattr"); maskattr; 
													maskattr = maskattr.next_sibling("maskattr")) {

		// Get the direction name such as "east"
      pugi::xml_attribute attr = anode.attribute("name");
      if (attr == nullptr) {
         errmsg << "Trait '" << getID() << "' maskattr node missing mandatory name field.";
         mudlog->writeLog(errmsg.str().c_str());
         return 0;
      }		
		std::string name = attr.value();

		if ((attr = anode.attribute("value") == nullptr) {

		}
		_altnames.push_back(attr.value());
   }

 
	return 1;
}

/*********************************************************************************************
 * **** functions to set trait attributes
 *
 *********************************************************************************************/

/* void Trait::setDesc(const char *newdesc) {
	_desc = newdesc;
}*/


