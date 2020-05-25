#ifndef DOOR_H
#define DOOR_H

#include <bitset>
#include <vector>
#include "Static.h"

class Location;

/***************************************************************************************
 * Door - A static object that creates a bridge between two locations. Can have a state, 
 *			 such as open, closed, locked 
 *
 ***************************************************************************************/
class Door : public Static 
{
public:
	
	// Constructors
	Door(const char *id); 
	Door(const Door &copy_from);

   virtual ~Door();

	enum doorflags { HideClosedExit };

   // Adds shared_ptr links between this object and others in the EntityDB. Polymorphic
   virtual void addLinks(EntityDB &edb, std::shared_ptr<Entity> self);

	std::shared_ptr<Entity> getCurLoc2() { return _cur_loc2; };
	virtual const char *getTitle() const { return _title.c_str(); };

	void setStartLoc2(const char *startloc);
	virtual void setTitle(const char *new_title) { _title = new_title; };

	bool isDoorFlagSet(doorflags the_flag) { return _doorflags[the_flag]; };
	
	std::shared_ptr<Entity> getOppositeLoc(std::shared_ptr<Entity> cur_loc);
   std::shared_ptr<Entity> getOppositeLoc(Location *cur_loc);


protected:

   virtual void saveData(pugi::xml_node &entnode) const;
   virtual int loadData(pugi::xml_node &entnode);

   virtual bool setFlagInternal(const char *flagname, bool newval);
   virtual bool isFlagSetInternal(const char *flagname, bool &results);

private:

	std::string _title;
	
	std::string _startloc2;
	std::shared_ptr<Entity> _cur_loc2;

	std::bitset<32> _doorflags;

	std::vector<std::string> _roomdesc;

};

#endif
