#ifndef ORGANISM_H
#define ORGANISM_H

#include "Entity.h"

/***************************************************************************************
 * Organism - a living entity that can usually be killed and sometimes moves around. Used
 *				  for players and NPCs. This is a generic class and should not be enstantiated
 *				  directly
 *
 ***************************************************************************************/
class Organism : public Entity 
{
public:
	
   virtual ~Organism();

protected:
	Organism(const char *id);	// Must be called from the child constructor
	Organism(const Organism &copy_from);

   virtual void saveData(pugi::xml_node &entnode) const;
   virtual int loadData(LogMgr &log, pugi::xml_node &entnode);

private:
};


#endif
