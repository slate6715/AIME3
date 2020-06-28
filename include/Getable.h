#ifndef GETABLE_H
#define GETABLE_H

#include <bitset>
#include <stack>
#include "Static.h"

/***************************************************************************************
 * Getable - An immobile object that is mainly used to provide descriptions or for basic
 *				interaction. A static is also a parent class of all interactable objects
 *
 ***************************************************************************************/
class Getable : public Static 
{
public:
	
	// Constructors
	Getable(const char *id); 
	Getable(const Getable &copy_from);

   virtual ~Getable();

	enum gflags { NoGet, NoDrop, Food, Rope, LuckFast, ThiefOnly, BlockMagic, FastHeal, EnhanceMagic, 
					CanLight };

	enum descstates { Pristine, Dropped, Lit, Extinguished, Open, Closed, Custom };


	// void setDesc(const char *newdesc);

	// Manages the roomdesc--the description one sees when they look in the room
	void setRoomDesc(descstates new_state, const char *new_desc, const char *customname = NULL);
	bool changeRoomDesc(descstates, const char *customname = NULL);
	const char *getRoomDesc();

   virtual const char *listContents(std::string &buf, const Physical *exclude = NULL) const;

   bool isGetableFlagSet(gflags flag) { return _getflags[flag]; };

   // Messing with fire!
   virtual bool light(std::string &errmsg);
   virtual bool extinguish(std::string &errmsg);

protected:

   virtual void saveData(pugi::xml_node &entnode) const;
   virtual int loadData(pugi::xml_node &entnode);

   virtual bool setFlagInternal(const char *flagname, bool newval);
   virtual bool isFlagSetInternal(const char *flagname, bool &results);

private:

	std::vector<std::pair<std::string, std::string> > _roomdesc;

	std::bitset<32> _getflags;

	unsigned int _dstate = (unsigned int) Pristine;
};

#endif
