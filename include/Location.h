#ifndef LOCATION_H
#define LOCATION_H

#include <bitset>
#include <vector>
#include "Entity.h"

class EntityDB;
class Location;

struct locexit {
	std::string dir;	
	std::string link_id;
	std::shared_ptr<Location>	link_loc;
	std::bitset<32> eflags;
};

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

	void setDesc(const char *newdesc);
   void setTitle(const char *newtitle);

	const char *getDesc() const { return _desc.c_str(); };
	const char *getTitle() const { return _title.c_str(); };

	std::shared_ptr<Location> getExit(const char *exitname);
   std::shared_ptr<Location> getExitAbbrev(const char *exitname);

   // Adds shared_ptr links between this object and others in the EntityDB. Polymorphic
   virtual void addLinks(EntityDB &edb);

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

#endif
