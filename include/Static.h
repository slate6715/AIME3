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

	enum sflags { NoneYet };

	void setDesc(const char *newdesc);
	void addAltName(const char *names);

	const char *getDesc() const { return _desc.c_str(); };

	bool hasAltName(const char *str);

protected:

   virtual void saveData(pugi::xml_node &entnode) const;
   virtual int loadData(pugi::xml_node &entnode);

   virtual bool setFlagInternal(const char *flagname, bool newval);
   virtual bool isFlagSetInternal(const char *flagname, bool &results);

private:
	
	std::string _desc;
	std::vector<std::string> _altnames;

	std::bitset<32> _staticflags;
};

#endif
