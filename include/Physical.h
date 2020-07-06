#ifndef PHYSICAL_H
#define PHYSICAL_H

#include <string>
#include <list>
#include <memory>
#include <map>
#include <vector>
#include <climits>
#include "../external/pugixml.hpp"
#include "Entity.h"
#include "Attribute.h"

class EntityDB;
class Organism;

/***************************************************************************************
 * Physical - Tangible objects in the MUD. Only child classes are enstantiated
 *
 ***************************************************************************************/
class Physical : public Entity
{
public:
   virtual ~Physical();

	std::shared_ptr<Physical> getPhysSelfPtr() { return std::dynamic_pointer_cast<Physical>(_self); };

	// Functions with represphysation in child classes
	virtual const char *getExamine() const = 0;
	virtual const char *getTitle() const = 0; 

   virtual const char *getGameName(std::string &buf) const { (void) buf; return NULL; };

	virtual const char *listContents(std::string &buf, const Physical *exclude = NULL) const 
																			{ (void) buf; (void) exclude; return NULL; };

	// Opens containers - non-overloaded functions raise an error here
	virtual bool open(std::string &errmsg);
	virtual bool close(std::string &errmsg);

	// add attributes of certain types to the list
   bool addAttribute(const char *attrib, int value);
   bool addAttribute(const char *attrib, float value);
   bool addAttribute(const char *attrib, const char *value);
   bool addAttributeUnk(const char *attrib, const char *value);

	// Remove an attribute from the list
	bool remAttribute(const char *attrib);

	// Setting and getting attribute values. These are less-efficiphys versions of the class
	// versions that use enum index as they have to lookup string to enums
	bool setAttribute(const char *attrib, int value);
	bool setAttribute(const char *attrib, float value);
	bool setAttribute(const char *attrib, const char *value);
   bool setAttribute(const char *attrib, Attribute &value);
	bool incrAttribute(const char *attrib, int increase, int max=0);

	int getAttribInt(const char *attrib);
	float getAttribFloat(const char *attrib);
	const char *getAttribStr(const char *attrib, std::string &buf);

	Attribute::attr_type getAttribType(const char *attrib) const;

	bool hasAttribute(const char *attrib);

   // Move an physity to a new container (removes from the old)
   bool movePhysical(std::shared_ptr<Physical> new_loc, std::shared_ptr<Physical> self = nullptr);

   // Add or remove objects to/from the container in this physity
   bool addPhysical(std::shared_ptr<Physical> new_phys);
   bool removePhysical(std::shared_ptr<Physical> new_phys);

   // Checks if this physity contains the parameter physity
   bool containsPhysical(std::shared_ptr<Physical> phys_ptr);

	// Does this physical contain something with the Static::Lit flag set
	std::shared_ptr<Physical> containsLit(int recursive_lvl = INT_MAX);
	std::shared_ptr<Physical> containsFlag(const char *flagname, int recursive_lvl = INT_MAX);
	std::shared_ptr<Physical> containsFlags(std::vector<std::string> &flaglist, int recursive_lvl = INT_MAX);

   // Retrieved the shared pointer matching the parameter information
   std::shared_ptr<Physical> getContainedByPtr(Physical *pptr);
	std::shared_ptr<Physical> getContainedByID(const char *id);
	virtual std::shared_ptr<Physical> getContainedByName(const char *name, bool allow_abbrev = true);

	std::shared_ptr<Physical> getCurLoc() { return _cur_loc; };

	// Adds shared_ptr links between this object and others in the PhysicalDB. Polymorphic
	virtual void addLinks(EntityDB &edb, std::shared_ptr<Physical> self) { (void) edb; (void) self; };

   virtual bool hasAltName(const char *str, bool allow_abbrev) 
													{ (void) str; (void) allow_abbrev; return false; };

	// Send a message to this physity or its contphyss - class-specific behavior
   virtual void sendMsg(const char *msg, std::shared_ptr<Physical> exclude=nullptr, std::shared_ptr<Physical> exclude2=nullptr) 
																				{ (void) msg; (void) exclude; (void) exclude2; };
   virtual void sendMsg(std::string &msg, std::shared_ptr<Physical> exclude=nullptr, std::shared_ptr<Physical> exclude2=nullptr) 
																				{ (void) msg; (void) exclude; (void) exclude2; };

	// Removes all references to the parameter from the Entities in the database so 
	// it can be safely removed
	virtual size_t purgePhysical(std::shared_ptr<Physical> item);

	int execSpecial(const char *trigger, std::vector<std::pair<std::string, std::shared_ptr<Physical>>> &variables,
												std::vector<std::pair<std::string, int>> *variable_ints = NULL, 
												std::vector<std::pair<std::string, float>> *variable_floats = NULL, 
												std::vector<std::pair<std::string, std::string>> *variable_strs = NULL);

	std::list<std::shared_ptr<Physical>>::const_iterator beginContained(){ return _contained.begin(); };
	std::list<std::shared_ptr<Physical>>::const_iterator endContained(){ return _contained.end(); };

	std::list<std::shared_ptr<Physical>>::iterator begin() { return _contained.begin(); };
	std::list<std::shared_ptr<Physical>>::iterator end() { return _contained.end(); };

protected:
	Physical(const char *id);	// Must be called from the child constructor
	Physical(const Physical &copy_from);

	virtual void saveData(pugi::xml_node &entnode) const;
	virtual int loadData(pugi::xml_node &entnode);

	virtual bool setFlagInternal(const char *flagname, bool newval);
	virtual bool isFlagSetInternal(const char *flagname, bool &results);

   virtual void fillAttrXMLNode(pugi::xml_node &anode) const;
	
   // All physicals can possibly contain objects
   std::list<std::shared_ptr<Physical>> _contained;

private:
	Physical();	// Should not be called

	std::shared_ptr<Physical> _cur_loc;
	
	std::vector<std::pair<std::string, std::string>> _specials;

	std::map<std::string, std::shared_ptr<Attribute>> _attributes;
};


#endif
