#ifndef STATIC_H
#define STATIC_H

#include <bitset>
#include <vector>
#include "Physical.h"

/***************************************************************************************
 * Static - An immobile object that is mainly used to provide descriptions or for basic
 *				interaction. A static is also a parent class of all interactable objects
 *
 ***************************************************************************************/
class Static : public Physical 
{
public:
	
	// Constructors
	Static(const char *id); 
	Static(const Static &copy_from);

   virtual ~Static();

	enum sflags { Container, Lockable, NotCloseable, Lightable, MagicLit, NoSummon };

   enum doorstate { Open, Closed, Locked, Special };

	void setExamine(const char *newexamine);
	void setStartLoc(const char *newloc);
	void addAltName(const char *names);
	
   // Gets the primary reference name the game refers to this entity by
	virtual const char *getGameName(std::string &buf) const;

	virtual const char *getTitle() const { return NULL; }; // No title for statics
	virtual const char *getExamine() const { return _examine.c_str(); };
	const char *getStartLoc() const { return _startloc.c_str(); };

	virtual bool hasAltName(const char *str, bool allow_abbrev);

   // Adds shared_ptr links between this object and others in the EntityDB. Polymorphic
   virtual void addLinks(EntityDB &edb, std::shared_ptr<Physical> self);

	virtual const char *listContents(std::string &buf, const Physical *exclude = NULL) const;

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
	
	std::string _examine;
	std::vector<std::string> _altnames;
	std::string _startloc;

	std::bitset<32> _staticflags;

   doorstate _state = Open;

   // List of key IDs that can unlock this door - no keys listed mean all keys work
   std::vector<std::string> _keys;

};

#endif
