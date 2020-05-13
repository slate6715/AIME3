#include <cstring>
#include <iostream>
#include <sstream>
#include "Entity.h"
#include "global.h"
#include "Attribute.h"


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

   for (pugi::xml_node flag = entnode.child("attribute"); flag; flag = flag.next_sibling("attribute")) {
      try {
         pugi::xml_attribute attr = flag.attribute("name");
         if (attr == nullptr) {
            errmsg << getTypeName() << " '" << getID() << "' attribute node missing mandatory name field.";
            mudlog->writeLog(errmsg.str().c_str());
            return 0;
         }
			std::string name = attr.value();

			attr = flag.attribute("value");
         if (attr == nullptr) {
            errmsg << getTypeName() << " '" << getID() << "' attribute node missing mandatory value field.";
            mudlog->writeLog(errmsg.str().c_str());
            return 0;
         }
			setAttribute(name.c_str(), attr.value());
      }
      catch (std::invalid_argument &e) {
         errmsg << getTypeName() << " '" << getID() << "' attribute error: " << e.what();
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
 * containsEntity - Checks if the entity is contained within this entity's container
 *
 *    Returns: true for contained, false otherwise
 *
 *********************************************************************************************/

bool Entity::containsEntity(std::shared_ptr<Entity> ent_ptr) {
	auto cptr = _contained.begin();
	for ( ; cptr != _contained.end(); cptr++) {
		if (ent_ptr == *cptr)
			return true;
	}
	return false;
}

/*********************************************************************************************
 * addEntity - adds the specified entity to the container
 *
 *    Returns: true if added, false if it was already contained
 *
 *********************************************************************************************/

bool Entity::addEntity(std::shared_ptr<Entity> new_ent) {
	if (containsEntity(new_ent))
		return false;

	_contained.push_back(new_ent);
	return true;
}

/*********************************************************************************************
 * removeEntity - Removes the entity from the container
 *
 *    Returns: true if found and removed, false if it was not found
 *
 *********************************************************************************************/

bool Entity::removeEntity(std::shared_ptr<Entity> ent_ptr) {
	auto cptr = _contained.begin();

   for ( ; cptr != _contained.end(); cptr++) {
      if (ent_ptr == *cptr){
         _contained.erase(cptr);
			return true;
		}
   }
   return false;
}

/*********************************************************************************************
 * moveEntity - Moves this entity to a new location, first removing it from the old locale and
 *					 planting it in the new one
 *
 *		Params:	new_ent - the new location for this object
 *					self - shared_ptr of itself - if null, will look in the _cur_loc for its shared
 *							 pointer
 *
 *    Returns: true if successful, false if there was an anomaly
 *
 *********************************************************************************************/

bool Entity::moveEntity(std::shared_ptr<Entity> new_ent, std::shared_ptr<Entity> self) {
	bool results = true;

	if (new_ent == nullptr) {
		throw std::runtime_error("Entity::moveEntity: attempted to set location to null Entity");
	}

	if (self == nullptr) {
		self = _cur_loc->getContained(this);
		if (self == nullptr){
			throw std::runtime_error("Entity::moveEntity: Could not retrieve self shared_ptr");
		}
	}
	if (_cur_loc != nullptr)
		results &= _cur_loc->removeEntity(self);

	_cur_loc = new_ent;

	results &= new_ent->addEntity(self);
	return results;
}

/*********************************************************************************************
 * getContained - retrieves the shared pointer of the contained entity based on a regular pointer
 *
 *    Returns: applicable shared pointer (which points to null if failed) 
 *
 *********************************************************************************************/

std::shared_ptr<Entity> Entity::getContained(Entity *eptr) {
   auto cptr = _contained.begin();

   for ( ; cptr != _contained.end(); cptr++) {
      if (eptr == &(**cptr)){
			return *cptr;
      }
   }
   return std::shared_ptr<Entity>(nullptr);
}

/*********************************************************************************************
 * setFlag, setFlagInternal - given the flag string, sets the flag or returns false if not found.
 *								Internal version calls up to parents to check their flags.
 *
 *		Throws: invalid_argument if the flag is not a valid flag
 *
 *********************************************************************************************/

void Entity::setFlag(const char *flagname, bool newval) {
	if (!setFlagInternal(flagname, newval)) {
		std::string msg("Unrecognized flag for entity class '");
		msg += flagname;
		msg += "'";
		throw std::invalid_argument(msg.c_str());
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

	if (!isFlagSetInternal(flagname, results)) {
      std::string msg("Unrecognized flag for entity class '");
      msg += flagname;
      msg += "'";
      throw std::invalid_argument(msg.c_str());
	}
	return results;
}

bool Entity::isFlagSetInternal(const char *flagname, bool &results) {
	// No entity flags yet
	(void) flagname; 
	(void) results;
	return false;
}


/*********************************************************************************************
 * getContained - returns a shared_ptr to the contained entity that matches name_alias. This
 *					   version only checks id
 *
 *    Returns: shared_ptr if found, nullptr if not 
 *
 *********************************************************************************************/

std::shared_ptr<Entity> Entity::getContained(const char *name_alias, bool allow_abbrev) {
   std::string name = name_alias;
	std::string buf;

   // First, look for the name
   auto eit = _contained.begin();
   for ( ; eit != _contained.end(); eit++) {
      if ((!allow_abbrev) && name.compare((*eit)->getID()))
         return *eit;
      else if ((allow_abbrev) && name.compare(0, name.size(), (*eit)->getNameID(buf)) == 0)
         return *eit;
   }
	return nullptr;

}

/*********************************************************************************************
 * purgeEntity - Removes all references to the parameter from the Entities in the database so
 *               it can be safely removed
 *
 *    Returns: number of references to this object cleared
 *
 *********************************************************************************************/

size_t Entity::purgeEntity(std::shared_ptr<Entity> item) {
	size_t count = 0;
	auto c_it = _contained.begin();

	// Look for contained items and remove it
	while (c_it != _contained.end()) {
		if ((*c_it) == item) {
			c_it = _contained.erase(c_it);
			count++;
		} else
			c_it++;
	}
	return count;
}

/*********************************************************************************************
 *	setAttribute, getAttribute - Setting and getting attribute values. These are less-efficient 
 *						versions of the class versions that use enum index as they have to lookup 
 *						string to enums
 *
 *
 *    Returns: true if found and set, false otherwise
 *
 *********************************************************************************************/
bool Entity::setAttribute(const char *attrib, int value) {
	return setAttribInternal(attrib, value);	
}

bool Entity::setAttribute(const char *attrib, float value) {
   return setAttribInternal(attrib, value);
}

// Special function also allows for string int and floats (converts internally)
bool Entity::setAttribute(const char *attrib, const char *value) {
   return setAttribInternal(attrib, value);
}

int Entity::getAttribInt(const char *attrib) {
	int value;
	if (!getAttribInternal(attrib, value)) {
		std::stringstream msg;
		msg << "Unknown attribute requested '" << attrib << "' in Entity '" << getID() << "'";
		throw std::invalid_argument(msg.str().c_str());
	}
	return value;
}

float Entity::getAttribFloat(const char *attrib) {
   float value;
   if (!getAttribInternal(attrib, value)) {
      std::stringstream msg;
      msg << "Unknown attribute requested '" << attrib << "' in Entity '" << getID() << "'";
      throw std::invalid_argument(msg.str().c_str());
   }
   return value;
}


const char *Entity::getAttribStr(const char *attrib, std::string &buf) {
   if (!getAttribInternal(attrib, buf)) {
      std::stringstream msg;
      msg << "Unknown attribute requested '" << attrib << "' in Entity '" << getID() << "'";
      throw std::invalid_argument(msg.str().c_str());
   }
   return buf.c_str();
}

/*********************************************************************************************          * set/getAttributeInternal - polymorphic version of the get/setAttribute function
 *
 *    Returns: true if found and set, false otherwise
 *
 *********************************************************************************************/        
bool Entity::setAttribInternal(const char *attrib, int value) {
	(void) attrib;
	(void) value;
	return false;
}

bool Entity::setAttribInternal(const char *attrib, float value) {
   (void) attrib;
   (void) value;
   return false;
}

bool Entity::setAttribInternal(const char *attrib, const char *value) {
   (void) attrib;
   (void) value;
   return false;
}

bool Entity::getAttribInternal(const char *attrib, int &value) {
   (void) attrib;
   (void) value;
   return false;
}

bool Entity::getAttribInternal(const char *attrib, float &value) {
   (void) attrib;
   (void) value;
   return false;
}

bool Entity::getAttribInternal(const char *attrib, std::string &value) {
   (void) attrib;
   (void) value;
   return false;
}


