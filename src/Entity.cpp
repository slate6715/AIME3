#include <cstring>
#include <iostream>
#include <sstream>
#include <regex>
#include "Entity.h"
#include "global.h"
#include "Attribute.h"
#include "misc.h"
#include "ScriptEngine.h"

/*********************************************************************************************
 * EntityDB (constructor) - Called by a child class to initialize any Entity elements
 *
 *********************************************************************************************/
Entity::Entity(const char *id):
								_id(id) 
{


}

// Called by child class
Entity::Entity(const Entity &copy_from):
								_id(copy_from._id)
{

}

// Mainly this code gets rid of effc++ warnings
Entity::Entity():
					_id("")
{
}


Entity::~Entity() {

}

/*********************************************************************************************
 * getNameID, getZoneID - Gets either just the name ID portion of the entity or the zonename
 *                        and copies the result into the passed in buffer
 *
 *********************************************************************************************/

const char *Entity::getNameID(std::string &buf) const {
	size_t amppos = _id.find(":");
	if (amppos == std::string::npos)
		throw std::runtime_error("EntityID was not in the correct format <zone>:<entityname>");

	buf = _id.substr(amppos+1, _id.size() - amppos);
	return buf.c_str();
}

const char *Entity::getZoneID(std::string &buf) const {
   size_t amppos = _id.find(":");
   if (amppos == std::string::npos)
      throw std::runtime_error("EntityID was not in the correct format <zone>:<entityname>");

   buf = _id.substr(0, amppos);
   return buf.c_str();

}

/*********************************************************************************************
 * setID - Changes the ID of this entity. Entity ID should be zone:name or something like
 *			  player:name. Changing this could cause duplicates or a mismatch with the key in
 *			  a std::map key. 
 *
 *********************************************************************************************/

void Entity::setID(const char *new_id) {
	if ((new_id == NULL) || (strchr(new_id, ':') == NULL))
		throw std::invalid_argument("Entity setID attempt to set to an invalid format. Must have an ':' in it");

	_id = new_id;
}

/*********************************************************************************************
 * saveData - Called by a child class to save Entity-specific data into an XML document
 *
 *		Params:	entnode - This entity's node within the XML tree so attributes can be added to
 *                       it. This should be set up by a child class
 *
 *********************************************************************************************/

void Entity::saveData(pugi::xml_node &entnode) const {

	// Add entity data
	pugi::xml_attribute idnode = entnode.append_attribute("id");
	idnode.set_value(_id.c_str());

}

/*********************************************************************************************
 * loadData - Called by a child class to populate Entity-specific data from an XML document
 *
 *    Params:  entnode - This entity's node within the XML tree so attributes can be drawn from
 *								 it
 *
 *		Returns: 1 for success, 0 for failure
 *
 *********************************************************************************************/

int Entity::loadData(pugi::xml_node &entnode) {

	pugi::xml_attribute attr = entnode.attribute("id");
	if (attr == nullptr) {
		mudlog->writeLog("Entity save file missing mandatory 'id' field.", 2);
		return 0;
	}
	_id = attr.value();

	std::stringstream errmsg;
	std::regex idcheck("\\w+:\\w+");
	if (!std::regex_match(_id, idcheck)) {
		errmsg << "ID '" << _id << "' invalid format. Should be <zone>:<id> where zone and ID are alphanumeric or _";
		mudlog->writeLog(errmsg.str().c_str());
		return 0;
	}

   // Get the Flags (if any)
   for (pugi::xml_node flag = entnode.child("flag"); flag; flag = flag.next_sibling("flag")) {
      try {
         pugi::xml_attribute attr = flag.attribute("name");
         if (attr == nullptr) {
            errmsg << getTypeName() << " '" << getID() << "' flag node missing mandatory name field.";
            mudlog->writeLog(errmsg.str().c_str());
            return 0;
         }
         setFlag(attr.value(), true);
      }
      catch (std::invalid_argument &e) {
         errmsg << getTypeName() << " '" << getID() << "' flag error: " << e.what();
         mudlog->writeLog(errmsg.str().c_str());
         return 0;
      }
   }

	return 1;

}

/*********************************************************************************************
 * loadEntity - Public entrypoint for all entities to load their class-specific details
 *
 *    Params:  entnode - This entity's node within the XML tree so attributes can be drawn from
 *                       it
 *
 *    Returns: 1 for success, 0 for failure
 *
 *********************************************************************************************/

int Entity::loadEntity(pugi::xml_node &enode) {
	return loadData(enode);
}


/*********************************************************************************************
 * setFlag, setFlagInternal - given the flag string, sets the flag or returns false if not found.
 *								Internal version calls up to parents to check their flags.
 *
 *		Throws: invalid_argument if the flag is not a valid flag
 *
 *********************************************************************************************/

void Entity::setFlag(const char *flagname, bool newval) {
	std::stringstream msg;
	if (!setFlagInternal(flagname, newval)) {
		msg << "Unrecognized flag '" << flagname << "' for entity class.";
		throw std::invalid_argument(msg.str().c_str());
	}
}

bool Entity::setFlagInternal(const char *flagname, bool newval) {
	// Does nothing right now
	(void) flagname;
	(void) newval;
	return false;
}

/*********************************************************************************************
 * isFlagSet, isFlagSetInternal - given the flag string, returns the flag setting for this entity
 *                      Internal version calls up to parents to check their flags.
 *
 *    Returns: true if flag is set, false otherwise
 *
 *    Throws: invalid_argument if the flag is not a valid flag
 *
 *********************************************************************************************/

bool Entity::isFlagSet(const char *flagname) {
	bool results;
	std::stringstream msg;

	if (!isFlagSetInternal(flagname, results)) {
		msg << "Unrecognized flag '" << flagname << "' for entity class.";
      throw std::invalid_argument(msg.str().c_str());
	}
	return results;
}

bool Entity::isFlagSetInternal(const char *flagname, bool &results) {
	// No entity flags yet
	(void) flagname; 
	(void) results;
	return false;
}


void Entity::setSelfPtr(std::shared_ptr<Entity> self) {
	if (self.get() != this)
		throw std::invalid_argument("setSelfPtr being assigned shared_ptr not affiliated with this Entity");
	_self = self;
}

/*********************************************************************************************
 * test for equality operator - determines if the rhs is the same instantiation as lhs
 *
 *    Returns: true if the same, false otherwise
 *
 *********************************************************************************************/
bool Entity::operator == (const Entity *rhs) {
	return (this == rhs);
}

bool Entity::operator == (std::shared_ptr<Entity> rhs) {
	return (this == &(*rhs));
}

bool Entity::operator == (const Entity &rhs) {
	return (*this == &rhs);
}


