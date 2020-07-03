#ifndef ENTITY_H
#define ENTITY_H

#include <string>
#include <list>
#include <memory>
#include <vector>
#include "../external/pugixml.hpp"

class Physical;

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

	// Checks if the other entity is pointing to the same instantiation
	virtual bool operator == (const Entity *rhs);
	virtual bool operator == (const Entity &rhs);
	virtual bool operator == (std::shared_ptr<Entity> rhs);

	const char *getID() const { return _id.c_str(); };
	const char *getNameID(std::string &buf) const;
	const char *getZoneID(std::string &buf) const;
	const char *getTypeName() const { return _typename.c_str(); };

	void setID(const char *new_id);	// Be careful setting this, must be unique ID

	int loadEntity(pugi::xml_node &enode);

	void setFlag(const char *flagname, bool newval);
	bool isFlagSet(const char *flagname);

	std::shared_ptr<Entity> getSelfPtr() const { return _self; };
	void setSelfPtr(std::shared_ptr<Entity> self);

   // Removes all references to the parameter from the Entities in the database so
   // it can be safely removed
   virtual size_t purgePhysical(std::shared_ptr<Physical> item) { (void) item; return 0; };


protected:
	Entity(const char *id);	// Must be called from the child constructor
	Entity(const Entity &copy_from);

	virtual void saveData(pugi::xml_node &entnode) const;
	virtual int loadData(pugi::xml_node &entnode);

	virtual bool setFlagInternal(const char *flagname, bool newval);
	virtual bool isFlagSetInternal(const char *flagname, bool &results);

	std::string _typename;

	std::shared_ptr<Entity> _self;

private:
	Entity();	// Should not be called

	std::string _id;
};


#endif
