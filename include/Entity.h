#ifndef ENTITY_H
#define ENTITY_H

#include <string>
#include <list>
#include <memory>
#include "../external/pugixml.hpp"

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

	int loadEntity(pugi::xml_node &enode);

	void setFlag(const char *flagname, bool newval);
	bool isFlagSet(const char *flagname);

   // Move an entity to a new container (removes from the old)
   bool moveEntity(std::shared_ptr<Entity> new_loc, std::shared_ptr<Entity> self = nullptr);

   // Add or remove objects to/from the container in this entity
   bool addEntity(std::shared_ptr<Entity> new_ent);
   bool removeEntity(std::shared_ptr<Entity> new_ent);

   // Checks if this entity contains the parameter entity
   bool containsEntity(std::shared_ptr<Entity> ent_ptr);

   // Retrieved the shared pointer matching the entity pointer
   std::shared_ptr<Entity> getContained(Entity *eptr);

	std::shared_ptr<Entity> getCurLoc() { return _cur_loc; };

protected:
	Entity(const char *id);	// Must be called from the child constructor
	Entity(const Entity &copy_from);

	virtual void saveData(pugi::xml_node &entnode) const;
	virtual int loadData(pugi::xml_node &entnode);

	virtual bool setFlagInternal(const char *flagname, bool newval);
	virtual bool isFlagSetInternal(const char *flagname, bool &results);

private:
	Entity();	// Should not be called

	std::string _id;

	// All entities can possibly contain objects
	std::list<std::shared_ptr<Entity>> _contained;

	std::shared_ptr<Entity> _cur_loc;
};


#endif
