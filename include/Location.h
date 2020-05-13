#ifndef LOCATION_H
#define LOCATION_H

#include <bitset>
#include <vector>
#include "Entity.h"

class EntityDB;
class Location;
class Player;

struct locexit;

/***************************************************************************************
 * Location - A location in the world that contains exits and can contain most other
 *				  game objects naturally.
 *
 ***************************************************************************************/
class Location : public Entity 
{
public:
	
	// Constructors
	Location(const char *id); 
	Location(const Location &copy_from);

   virtual ~Location();

	enum lflags {Outdoors, Bright, Death};
	enum exitflags {Hidden, Special};
	enum exitdirs {North, South, East, West, Up, Down, Northeast, Northwest, Southeast, Southwest, 
					   Custom};

	void setDesc(const char *newdesc);
   void setTitle(const char *newtitle);

	const char *getDesc() const { return _desc.c_str(); };
	const char *getTitle() const { return _title.c_str(); };
	virtual const char *listContents(std::string &buf, Player *exclude);

	std::shared_ptr<Location> getExit(const char *exitname);
   std::shared_ptr<Location> getExitAbbrev(std::string &exitname, exitdirs *val = NULL);

   // Adds shared_ptr links between this object and others in the EntityDB. Polymorphic
   virtual void addLinks(EntityDB &edb, std::shared_ptr<Entity> self);

	// Assembles a formatted list of the visible exits
	const char *getExitsStr(std::string &buf);

   virtual std::shared_ptr<Entity> getContained(const char *name_alias, bool allow_abbrev=true);

   // Send a message to this entity or its contents - class-specific behavior
   virtual void sendMsg(const char *msg, std::shared_ptr<Entity> exclude=nullptr); 
   virtual void sendMsg(std::string &msg, std::shared_ptr<Entity> exclude=nullptr); 

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
   std::shared_ptr<Location>  link_loc;
   std::bitset<32> eflags;
};

#endif
