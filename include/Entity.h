#ifndef ENTITY_H
#define ENTITY_H

#include <string>
#include "../external/pugixml.hpp"
#include "LogMgr.h"

/***************************************************************************************
 * Entity - the most abstract class of interactable MUD objects. This is a generic class
 *				so you would never create an Entity object. Hence, constructors are protected
 *				and there are some generic methods. 
 *
 ***************************************************************************************/
class Entity 
{
public:
   virtual ~Entity();

	const char *getID() const { return _id.c_str(); };
	const char *getNameID(std::string &buf) const;
	const char *getZoneID(std::string &buf) const;

	void setID(const char *new_id);	// Be careful setting this, must be unique ID

protected:
	Entity(const char *id);	// Must be called from the child constructor
	Entity(const Entity &copy_from);

	virtual void saveData(pugi::xml_node &entnode) const;
	virtual int loadData(LogMgr &log, pugi::xml_node &entnode);

private:
	Entity();	// Should not be called

	std::string _id;
};


#endif
