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

	enum gflags { NoGet };

	// void setDesc(const char *newdesc);

	// virtual const char *getDesc() const { return _desc.c_str(); };
   void setTitle(const char *newtitle);


	// Manages the roomdesc--the description one sees when they look in the room
	void pushRoomDesc(const char *new_desc);
	void popRoomDesc();
	const char *getRoomDesc();
   const char *getTitle() const { return _title.c_str(); };

protected:

   virtual void saveData(pugi::xml_node &entnode) const;
   virtual int loadData(pugi::xml_node &entnode);

   virtual bool setFlagInternal(const char *flagname, bool newval);
   virtual bool isFlagSetInternal(const char *flagname, bool &results);

private:

	std::string _title;
	std::stack<std::string> _roomdesc;	

	std::bitset<32> _getflags;
};

#endif