#ifndef LOCATION_H
#define LOCATION_H

#include <bitset>
#include <vector>
#include "Physical.h"

class EntityDB;
class Location;
class Player;

struct locexit;

/***************************************************************************************
 * Location - A location in the world that contains exits and can contain most other
 *				  game objects naturally.
 *
 ***************************************************************************************/
class Location : public Physical 
{
public:
	
	// Constructors
	Location(const char *id); 
	Location(const Location &copy_from);

   virtual ~Location();

	enum lflags {Outdoors, Bright, Death, Realtime, NoMobiles, Dark, NoSummon, Private, OnePerson,
					 NoTeleport, Peaceful, Maze, Soundproof };
	enum exitflags {Hidden, Special};
	enum exitdirs {North, South, East, West, Up, Down, Northeast, Northwest, Southeast, Southwest, 
					   Custom};

	void setDesc(const char *newdesc);
   void setTitle(const char *newtitle);

	virtual const char *getDesc() const { return _desc.c_str(); };
	virtual const char *getExamine() const { return _desc.c_str(); };
	virtual const char *getTitle() const { return _title.c_str(); };
   virtual const char *listContents(std::string &buf, const Physical *exclude = NULL) const;

	bool isLocFlagSet(lflags flag) { return _locflags[flag]; };

	std::shared_ptr<Physical> getExit(const char *exitname);
   std::shared_ptr<Physical> getExitAbbrev(std::string &exitname, exitdirs *val = NULL);

	bool setExit(const char *exitname, std::shared_ptr<Physical> new_exit);
	bool clrExit(const char *exitname);

   // Adds shared_ptr links between this object and others in the EntityDB. Polymorphic
   virtual void addLinks(EntityDB &edb, std::shared_ptr<Physical> self);

	// Assembles a formatted list of the visible exits
	const char *getExitsStr(std::string &buf);

   // Send a message to this entity or its contents - class-specific behavior
   virtual void sendMsg(const char *msg, std::shared_ptr<Physical> exclude=nullptr, std::shared_ptr<Physical> exclude2=nullptr); 
   virtual void sendMsg(std::string &msg, std::shared_ptr<Physical> exclude=nullptr, std::shared_ptr<Physical> exclude2=nullptr); 

	static exitdirs getOppositeDir(exitdirs dir);

protected:

   virtual void saveData(pugi::xml_node &entnode) const;
   virtual int loadData(pugi::xml_node &entnode);

   virtual bool setFlagInternal(const char *flagname, bool newval);
   virtual bool isFlagSetInternal(const char *flagname, bool &results);

private:
	
	std::string _desc;
	std::string _title;

	std::bitset<32> _locflags;

	std::vector<locexit> _exits;
};


struct locexit {
   std::string dir;
   std::string link_id;
   Location::exitdirs exitval;
   std::shared_ptr<Physical>  link_loc;
   std::bitset<32> eflags;
};

#endif
