#ifndef STATIC_H
#define STATIC_H

#include <bitset>
#include <vector>
#include "Entity.h"

/***************************************************************************************
 * Static - An immobile object that is mainly used to provide descriptions or for basic
 *				interaction. A static is also a parent class of all interactable objects
 *
 ***************************************************************************************/
class Static : public Entity 
{
public:
	
	// Constructors
	Static(const char *id); 
	Static(const Static &copy_from);

   virtual ~Static();

	enum sflags { Container, Lockable, Closeable, Lightable, MagicLit };

   enum doorstate { Open, Closed, Locked, Special };

	void setDesc(const char *newdesc);
	void setStartLoc(const char *newloc);
	void addAltName(const char *names);

	virtual const char *getDesc() const { return _desc.c_str(); };
	const char *getStartLoc() const { return _startloc.c_str(); };

	virtual bool hasAltName(const char *str, bool allow_abbrev);

   // Adds shared_ptr links between this object and others in the EntityDB. Polymorphic
   virtual void addLinks(EntityDB &edb, std::shared_ptr<Entity> self);

	virtual std::shared_ptr<Entity> getContained(const char *name_alias, bool allow_abbrev=true);

	virtual const char *listContents(std::string &buf) const;

   doorstate getDoorState() { return _state; };
   void setDoorState(doorstate new_state) { _state = new_state; };
   bool setDoorState(const char *new_state);
	
	bool isStaticFlagSet(sflags flag) { return _staticflags[flag]; };

   // Opens containers - non-overloaded functions raise an error here
   virtual bool open(std::string &errmsg);
   virtual bool close(std::string &errmsg);

protected:

   virtual void saveData(pugi::xml_node &entnode) const;
   virtual int loadData(pugi::xml_node &entnode);

   virtual bool setFlagInternal(const char *flagname, bool newval);
   virtual bool isFlagSetInternal(const char *flagname, bool &results);

private:
	
	std::string _desc;
	std::vector<std::string> _altnames;
	std::string _startloc;

	std::bitset<32> _staticflags;

   doorstate _state = Open;

   // List of key IDs that can unlock this door - no keys listed mean all keys work
   std::vector<std::string> _keys;

};

#endif
