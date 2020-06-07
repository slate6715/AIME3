#include <iostream>
#include <sstream>
#include <memory>
#include "Trait.h"
#include "Attribute.h"
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

		// Get the attribute name to be masked
      pugi::xml_attribute attr = maskattr.attribute("name");
      if (attr == nullptr) {
         errmsg << "Trait '" << getID() << "' maskattr node missing mandatory name field.";
         mudlog->writeLog(errmsg.str().c_str());
         return 0;
      }		
		std::string name = attr.value();

      // Get the action taken when adjusting the attribute (set, add, multiply)
      attr = maskattr.attribute("action");
      if (attr == nullptr) {
         errmsg << "Trait '" << getID() << "' maskattr node missing mandatory action field.";
         mudlog->writeLog(errmsg.str().c_str());
         return 0;
      }
      std::string actionstr = attr.value();

		mask_action action = Set;
		lower(actionstr);
		if (actionstr.compare("set") == 0)
			action = Set;
		else if (actionstr.compare("add") == 0)
			action = Add;
		else if (actionstr.compare("multiply") == 0)
			action = Multiply;
		else {
         errmsg << "Trait '" << getID() << "' maskattr node action attribute '" << actionstr << 
																											"' not a valid action.";
         mudlog->writeLog(errmsg.str().c_str());
         return 0;
		}

		if ((attr = maskattr.attribute("value")) == nullptr) {
         errmsg << "Trait '" << getID() << "' maskattr node missing mandatory value field.";
         mudlog->writeLog(errmsg.str().c_str());
         return 0;
		}
		std::string value(attr.value());

		// We need to determine the type--int, float, or string. If they specify, we go with that. 
		// Otherwise, predict from the string itself
		Attribute *new_attr = NULL;
		try {
			if ((attr = maskattr.attribute("type")) != nullptr) {
				std::string typestr(attr.value());
				lower(typestr);

				if (typestr.compare("int") == 0) {
					new_attr = new IntAttribute(value.c_str());
				} else if (typestr.compare("float") == 0) {
					new_attr = new FloatAttribute(value.c_str());
				} else if (typestr.compare("str") == 0) {
					new_attr = new StrAttribute(value.c_str());
				} else {
		         errmsg << "Trait '" << getID() << "' maskattr node type attribute '" << typestr <<
                                                                                 "' not a valid type.";
				   mudlog->writeLog(errmsg.str().c_str());
					return 0;
				}
			} else {
				// Else we try to predict the value type (no need to check null, will throw exception)
				new_attr = genAttrFromStr(value.c_str());
			}
		} catch (std::invalid_argument &e) {
         errmsg << "Trait '" << getID() << "' maskattr node value invalid format: " << e.what();
         mudlog->writeLog(errmsg.str().c_str());
         return 0;
		} catch (std::out_of_range &e) {
         errmsg << "Trait '" << getID() << "' maskattr node value out of range: " << e.what();
         mudlog->writeLog(errmsg.str().c_str());
         return 0;
		}

		_init_list.push_back(std::pair<mask_action, std::unique_ptr<Attribute>>(action, std::unique_ptr<Attribute>(new_attr)));
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


