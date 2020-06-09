#ifndef EQUIPMENT_H
#define EQUIPMENT_H

#include <bitset>
#include <stack>
#include "Getable.h"

/***************************************************************************************
 * Equipment - A getable object that can be wielded in one or both hands
 *
 ***************************************************************************************/
class Equipment : public Getable 
{
public:
	
	// Constructors
	Equipment(const char *id); 
	Equipment(const Equipment &copy_from);

   virtual ~Equipment();

	enum equipflags { TwoHanded, AntiMagic };

	// void setDesc(const char *newdesc);

	// virtual const char *getDesc() const { return _desc.c_str(); };
   // void setTitle(const char *newtitle);


   // const char *getTitle() const { return _title.c_str(); };

   // Adds shared_ptr links between this object and others in the EntityDB. Polymorphic
   virtual void addLinks(EntityDB &edb, std::shared_ptr<Entity> self);

	size_t getEquipListSize() const { return _equip_list.size(); };
	const char *getEquipListName(size_t index) const { return _equip_list[index].first.c_str(); };
	const char *getEquipListGroup(size_t index) const { return _equip_list[index].second.c_str(); };

protected:

   virtual void saveData(pugi::xml_node &entnode) const;
   virtual int loadData(pugi::xml_node &entnode);

   virtual bool setFlagInternal(const char *flagname, bool newval);
   virtual bool isFlagSetInternal(const char *flagname, bool &results);

private:

	// Stores target info for linking to body parts
	std::vector<std::pair<std::string, std::string>> _equip_list;

	std::bitset<32> _equipflags;

	bool _start_equipped = false;
};

#endif
