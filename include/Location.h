#ifndef LOCATION_H
#define LOCATION_H

#include <bitset>
#include "Entity.h"


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

	enum lflags {Outdoors};

	void setDesc(const char *newdesc);
   void setTitle(const char *newtitle);

	const char *getDesc() const { return _desc.c_str(); };
	const char *getTitle() const { return _title.c_str(); };

protected:

   virtual void saveData(pugi::xml_node &entnode) const;
   virtual int loadData(pugi::xml_node &entnode);

   virtual bool setFlagInternal(const char *flagname, bool newval);
   virtual bool isFlagSetInternal(const char *flagname, bool &results);

private:
	
	std::string _desc;
	std::string _title;

	std::bitset<32> _locflags;

};

#endif
