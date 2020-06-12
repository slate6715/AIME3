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

	// Save attributes (polymorphic)
	fillAttrXMLNode(entnode);
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

	for (pugi::xml_node special = entnode.child("special"); special; special = special.next_sibling("special")) {
		try {
			pugi::xml_attribute attr = special.attribute("trigger");
         if (attr == nullptr) {
            errmsg << getTypeName() << " '" << getID() << "' special node missing mandatory trigger field.";
            mudlog->writeLog(errmsg.str().c_str());
            return 0;
         }
         std::string trigger = attr.value();

			_specials.push_back(std::pair<std::string, std::string>(trigger, special.child_value()));
		}
      catch (std::invalid_argument &e) {
         errmsg << getTypeName() << " '" << getID() << "' specials error: " << e.what();
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
			ent_ptr->_cur_loc = nullptr;
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

	if ((_cur_loc != nullptr) && (self == nullptr)) {
		self = _cur_loc->getContainedByPtr(this);
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
 * getContainedByPtr - retrieves the shared pointer of the contained entity based on a regular pointer
 *
 *    Returns: applicable shared pointer (which points to null if failed) 
 *
 *********************************************************************************************/

std::shared_ptr<Entity> Entity::getContainedByPtr(Entity *eptr) {
   auto cptr = _contained.begin();

   for ( ; cptr != _contained.end(); cptr++) {
      if (eptr == &(**cptr)){
			return *cptr;
      }
   }
   return std::shared_ptr<Entity>(nullptr);
}

/*********************************************************************************************
 * getContainedByID - returns a shared_ptr to the contained entity that matches name_alias. This
 *                   version only checks id
 *
 *    Returns: shared_ptr if found, nullptr if not
 *
 *********************************************************************************************/

std::shared_ptr<Entity> Entity::getContainedByID(const char *name_alias) {
   std::string name = name_alias;

   // Search through the contained for the object
   auto eit = _contained.begin();
   for ( ; eit != _contained.end(); eit++) {

      if (name.compare(eit->get()->getID()) == 0)
         return *eit;
   }
   return nullptr;

}

/*********************************************************************************************
 * getContainedByName - returns a shared_ptr to the contained entity that matches the name.
 *					Polymorphic. For this class version, only matches the game name field which varies
 *             depending on the entity class
 *
 *		Params:	name - string to search the NameID for
 *					allow_abbrev - if true, entity only needs to match up to sizeof(name)
 *
 *    Returns: shared_ptr if found, nullptr if not
 *
 *********************************************************************************************/

std::shared_ptr<Entity> Entity::getContainedByName(const char *name, bool allow_abbrev) {
   std::string namebuf = name;
	std::string ebuf;

   // Search through the contained for the object
   auto eit = _contained.begin();
   for ( ; eit != _contained.end(); eit++) {

		// Get the Game name which might be Title or NameID
		eit->get()->getGameName(ebuf);
      if ((!allow_abbrev) && (namebuf.compare(ebuf) == 0))
         return *eit;
		else if ((allow_abbrev) && equalAbbrev(namebuf, ebuf.c_str()))
			return *eit;
   }
   return nullptr;

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

// Special function also allows for string int and floats (converts internally)
bool Entity::setAttribute(const char *attrib, Attribute &value) {
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

/*********************************************************************************************          
 * set/getAttributeInternal - polymorphic version of the get/setAttribute function
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

bool Entity::setAttribInternal(const char *attrib, Attribute &value) {
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

/*********************************************************************************************
 * getAttribType - locates the attribute by its string and returns the type.
 *                   calls a polymorphic version getAttribTypeInternal
 *
 *    Returns: attr_type of Int, Float, Str, or Undefined if not found
 *
 *********************************************************************************************/

Attribute::attr_type Entity::getAttribType(const char *attrib) const {
   return getAttribTypeInternal(attrib);
}

Attribute::attr_type Entity::getAttribTypeInternal(const char *attrib) const {
	(void) attrib;

   // No Entity-level attributes yet
   return Attribute::Undefined;
}


/*********************************************************************************************
 * fillAttrXMLNode - populates the parameter XML node with data from this entity's attributes. 
 *                   polymorphic
 *
 *    Returns: true if the same, false otherwise
 *
 *********************************************************************************************/

void Entity::fillAttrXMLNode(pugi::xml_node &anode) const {
	// Currently nothing in the Entity parent class but might add cur_loc and contained
	(void) anode;	
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

/*********************************************************************************************
 * open, close - open or close a container or door - this is the default function for objects
 *				     that cannot be opened or closed. 
 *
 *********************************************************************************************/

bool Entity::open(std::string &errmsg) {
	errmsg = "You can't open that.\n";
	return false;
}

bool Entity::close(std::string &errmsg) {
	errmsg = "You can't close that.\n";
	return false;
}

/*********************************************************************************************
 * execSpecial - looks for the given trigger attached to this entity and executes it if found.
 *               
 *		Params:	trigger - the trigger string to match
 *					actor - the organism executing this action
 *
 *		Returns: -1 for error
 *					0 indicates the trigger was not found attached to this entity
 *					1 indicates the trigger executed and the command process should proceed normally
 *					2 indicates the trigger executed and the command process should terminate
 *
 *********************************************************************************************/

int Entity::execSpecial(const char *trigger, std::shared_ptr<Organism> actor) {

	// Loop through all the specials attached to this object
	for (unsigned int i=0; i<_specials.size(); i++) {
		
		// Compare special against trigger
		if (_specials[i].first.compare(trigger) == 0)
		{
			ScriptEngine script(_specials[i].second);

			script.setActor(actor);

			script.execute();

			return 1;
		}
	}

	// No special found with that trigger, exit
	return 0;
}


