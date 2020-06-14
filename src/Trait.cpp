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

		_init_list.emplace_back(attr_mask(name, action, new_attr));
   }

 
	return 1;
}

/*********************************************************************************************
 * maskPlayer - modifies the player's stats based on the "mask" of this trait
 *
 *
 *********************************************************************************************/

void Trait::maskPlayer(std::shared_ptr<Player> plr) {
	std::stringstream errmsg;

	// Loop through our initialization list
	for (unsigned int i=0; i<_init_list.size(); i++) {
		Attribute::attr_type atype = plr->getAttribType(_init_list[i].name.c_str());

		// We did not find the specified attribute to mask, error
		if (atype == Attribute::Undefined) {
			errmsg << "Trait '" << getID() << "' player attribute '" << _init_list[i].name << 
													"' doesn't seem to exist and can't be masked.";
			mudlog->writeLog(errmsg.str().c_str());
			continue;
		}

		// If the field is a string, we can only set, not add or multiply
		if (atype == Attribute::String) {
			if ((_init_list[i].action == Add) || (_init_list[i].action == Multiply) || (_init_list[i].attr->getType() != Attribute::String)) {
				errmsg << "Trait '" << getID() << "' attribute '" << _init_list[i].name << 
										"' is a string. The only mask action allowed is to Set by another String type.";
				mudlog->writeLog(errmsg.str().c_str());
				continue;
			}

			// Set the attribute
			plr->Physical::setAttribute(_init_list[i].name.c_str(), _init_list[i].attr->getStr());
			continue;
		}

		// Else it's either a float or an int--if the action is to Set, just do it
		if (_init_list[i].action == Set) {
			if (_init_list[i].attr->getType() != atype) {
            errmsg << "Trait '" << getID() << "' attribute '" << _init_list[i].name <<
                              "' type mismatch for Set action. Mask and attribute type must be the same.";
            mudlog->writeLog(errmsg.str().c_str());
            continue;
			}
			
			plr->Physical::setAttribute(_init_list[i].name.c_str(), *(_init_list[i].attr));
			continue;
		}
		
		// Finally, the action is either add or multiply
		if (atype == Attribute::Int) {
			IntAttribute newval, value = plr->getAttribInt(_init_list[i].name.c_str());
			if (_init_list[i].action == Add)
				newval = value + *(_init_list[i].attr);
			else
				newval = value * *(_init_list[i].attr);
			plr->Physical::setAttribute(_init_list[i].name.c_str(), newval);
		} else {
			FloatAttribute newval, value = plr->getAttribFloat(_init_list[i].name.c_str());
         if (_init_list[i].action == Add)
            newval = value + *(_init_list[i].attr);
         else
            newval = value * *(_init_list[i].attr);
         plr->Physical::setAttribute(_init_list[i].name.c_str(), newval);
		}
	}
}


/*********************************************************************************************
 * **** functions to set trait attributes
 *
 *********************************************************************************************/

/* void Trait::setDesc(const char *newdesc) {
	_desc = newdesc;
}*/


