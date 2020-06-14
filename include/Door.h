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

	enum doorflags { HideClosedExit, RopeDoor };

   // Adds shared_ptr links between this object and others in the EntityDB. Polymorphic
   virtual void addLinks(EntityDB &edb, std::shared_ptr<Physical> self);

	std::shared_ptr<Physical> getCurLoc2() { return _cur_loc2; };
	virtual const char *getTitle() const { return _title.c_str(); };
	const char *getExamine2() const { return _examine2.c_str(); };

	virtual const char *getGameName(std::string &buf) const;

	void setStartLoc2(const char *startloc);
	virtual void setTitle(const char *new_title) { _title = new_title; };
	void setExamine2(const char *new_examine) { _examine2 = new_examine; };

	bool isDoorFlagSet(doorflags the_flag) { return _doorflags[the_flag]; };
	
	std::shared_ptr<Physical> getOppositeLoc(std::shared_ptr<Physical> cur_loc);
   std::shared_ptr<Physical> getOppositeLoc(Location *cur_loc);

	const char *getCurRoomdesc(const Location *cur_loc);

   // Opens and closes the door (if it is not a rope door)
   virtual bool open(std::string &errmsg);
   virtual bool close(std::string &errmsg);

	// Set of open and close functions for a door using a tool, such as a rope
   virtual bool open(std::shared_ptr<Physical> tool, std::string &errmsg);
   std::shared_ptr<Physical> closeTool(std::string &errmsg);

protected:

   virtual void saveData(pugi::xml_node &entnode) const;
   virtual int loadData(pugi::xml_node &entnode);

   virtual bool setFlagInternal(const char *flagname, bool newval);
   virtual bool isFlagSetInternal(const char *flagname, bool &results);

private:

	std::string _title;

	std::string _examine2;
	
	std::string _startloc2;
	std::shared_ptr<Physical> _cur_loc2;

	std::bitset<32> _doorflags;

	std::vector<std::string> _roomdesc;
   std::vector<std::string> _roomdesc2;
};

#endif
