#include <cstring>
#include "Entity.h"


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
	size_t amppos = _id.find("@");
	if (amppos == std::string::npos)
		throw std::runtime_error("EntityID was not in the correct format <zone>@<entityname>");

	buf = _id.substr(amppos+1, _id.size() - amppos);
	return buf.c_str();
}

const char *Entity::getZoneID(std::string &buf) const {
   size_t amppos = _id.find("@");
   if (amppos == std::string::npos)
      throw std::runtime_error("EntityID was not in the correct format <zone>@<entityname>");

   buf = _id.substr(0, amppos);
   return buf.c_str();

}

/*********************************************************************************************
 * setID - Changes the ID of this entity. Entity ID should be zone@name or something like
 *			  player@name. Changing this could cause duplicates or a mismatch with the key in
 *			  a std::map key. 
 *
 *********************************************************************************************/

void Entity::setID(const char *new_id) {
	if ((new_id == NULL) || (strchr(new_id, '@') == NULL))
		throw std::invalid_argument("Entity setID attempt to set to an invalid format. Must have an '@' in it");

	_id = new_id;
}

/*********************************************************************************************
 * saveData - Called by a child class to save Entity-specific data into an XML document
 *
 *		Params:	entnode - This entity's node within the XML tree so attributes can be added to
 *                       it. This should be set up by a child class
 *					log - to log any errors
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
 *					log - to log any errors
 *
 *		Returns: 1 for success, 0 for failure
 *
 *********************************************************************************************/

int Entity::loadData(LogMgr &log, pugi::xml_node &entnode) {

	pugi::xml_attribute attr = entnode.attribute("id");
	if (attr == nullptr) {
		log.writeLog("Entity save file missing mandatory 'id' field.", 2);
		return 0;
	}
	_id = attr.value();
	return 1;

}


