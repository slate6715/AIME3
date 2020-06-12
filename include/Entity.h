#ifndef ENTITY_H
#define ENTITY_H

#include <string>
#include <list>
#include <memory>
#include <vector>
#include "../external/pugixml.hpp"
#include "Attribute.h"

class EntityDB;
class Organism;

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

	virtual const char *getGameName(std::string &buf) { (void) buf; return NULL; }; 

	// Functions with representation in child classes
	virtual const char *getDesc() const { return NULL; };
	virtual const char *getTitle() const { return NULL; };

	virtual const char *listContents(std::string &buf, const Entity *exclude = NULL) const 
																			{ (void) buf; (void) exclude; return NULL; };

	// Opens containers - non-overloaded functions raise an error here
	virtual bool open(std::string &errmsg);
	virtual bool close(std::string &errmsg);

	void setID(const char *new_id);	// Be careful setting this, must be unique ID

	int loadEntity(pugi::xml_node &enode);

	void setFlag(const char *flagname, bool newval);
	bool isFlagSet(const char *flagname);

	// Setting and getting attribute values. These are less-efficient versions of the class
	// versions that use enum index as they have to lookup string to enums
	bool setAttribute(const char *attrib, int value);
	bool setAttribute(const char *attrib, float value);
	bool setAttribute(const char *attrib, const char *value);
   bool setAttribute(const char *attrib, Attribute &value);

	int getAttribInt(const char *attrib);
	float getAttribFloat(const char *attrib);
	const char *getAttribStr(const char *attrib, std::string &buf);

	Attribute::attr_type getAttribType(const char *attrib) const;

   // Move an entity to a new container (removes from the old)
   bool moveEntity(std::shared_ptr<Entity> new_loc, std::shared_ptr<Entity> self = nullptr);

   // Add or remove objects to/from the container in this entity
   bool addEntity(std::shared_ptr<Entity> new_ent);
   bool removeEntity(std::shared_ptr<Entity> new_ent);

   // Checks if this entity contains the parameter entity
   bool containsEntity(std::shared_ptr<Entity> ent_ptr);

   // Retrieved the shared pointer matching the parameter information
   std::shared_ptr<Entity> getContainedByPtr(Entity *eptr);
	std::shared_ptr<Entity> getContainedByID(const char *id);
	virtual std::shared_ptr<Entity> getContainedByName(const char *name, bool allow_abbrev = true);

	std::shared_ptr<Entity> getCurLoc() { return _cur_loc; };

	// Adds shared_ptr links between this object and others in the EntityDB. Polymorphic
	virtual void addLinks(EntityDB &edb, std::shared_ptr<Entity> self) { (void) edb; (void) self; };

   virtual bool hasAltName(const char *str, bool allow_abbrev) 
													{ (void) str; (void) allow_abbrev; return false; };

	// Send a message to this entity or its contents - class-specific behavior
   virtual void sendMsg(const char *msg, std::shared_ptr<Entity> exclude=nullptr) 
																				{ (void) msg; (void) exclude; };
   virtual void sendMsg(std::string &msg, std::shared_ptr<Entity> exclude=nullptr) 
																				{ (void) msg; (void) exclude; };

	// Removes all references to the parameter from the Entities in the database so 
	// it can be safely removed
	virtual size_t purgeEntity(std::shared_ptr<Entity> item);

	int execSpecial(const char *trigger, std::shared_ptr<Organism> actor);

protected:
	Entity(const char *id);	// Must be called from the child constructor
	Entity(const Entity &copy_from);

	virtual void saveData(pugi::xml_node &entnode) const;
	virtual int loadData(pugi::xml_node &entnode);

	virtual bool setFlagInternal(const char *flagname, bool newval);
	virtual bool isFlagSetInternal(const char *flagname, bool &results);

	virtual bool setAttribInternal(const char *attrib, int value);
	virtual bool setAttribInternal(const char *attrib, float value);
	virtual bool setAttribInternal(const char *attrib, const char *value);
   virtual bool setAttribInternal(const char *attrib, Attribute &value);

	virtual bool getAttribInternal(const char *attrib, int &value);
	virtual bool getAttribInternal(const char *attrib, float &value);
	virtual bool getAttribInternal(const char *attrib, std::string &value);

	virtual Attribute::attr_type getAttribTypeInternal(const char *attrib) const;

   virtual void fillAttrXMLNode(pugi::xml_node &anode) const;
	
   // All entities can possibly contain objects
   std::list<std::shared_ptr<Entity>> _contained;

	std::string _typename;

private:
	Entity();	// Should not be called

	std::string _id;

	std::shared_ptr<Entity> _cur_loc;
	
	std::vector<std::pair<std::string, std::string>> _specials;
};


#endif
