#ifndef TRAIT_H
#define TRAIT_H

#include <chrono>
#include <memory>
#include <bitset>
#include <vector>
#include "Entity.h"

class MUD;
class UserMgr;
class TraitComponent;  // Defined afterwards

/***************************************************************************************
 * Trait - A trait is something that can be assigned to an organism and is mutally-exclusive
 *			  The actual trait that is assigned is a TraitComponent, such as orc is a component
 *         of the trait race
 *
 ***************************************************************************************/

class TraitComponent;  // Defined afterwards

class Trait : public Entity 
{
public:
	
	// Constructors
	Trait(const char *id); 
	Trait(const Trait &copy_from);

   virtual ~Trait();

   enum mask_action { Set, Add, Multiply };

   void maskPlayer(std::shared_ptr<Player> plr);


protected:

   virtual void saveData(pugi::xml_node &entnode) const;
   virtual int loadData(pugi::xml_node &entnode);

   virtual bool setFlagInternal(const char *flagname, bool newval);
   virtual bool isFlagSetInternal(const char *flagname, bool &results);

private:

	// Used to define starting attributes of this trait to simplify character creation
	std::vector<std::pair<mask_action, Attribute>> _init_list;

	
};

#endif
