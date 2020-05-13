#ifndef NPC_H
#define NPC_H

#include "Organism.h"

/***************************************************************************************
 * NPC - a living entity that can usually be killed and sometimes moves around. Used
 *				  for players and NPCs. This is a generic class and should not be enstantiated
 *				  directly
 *
 ***************************************************************************************/
class NPC : public Organism
{
public:
   NPC(const char *id);  // Must be called from the child constructor
   NPC(const NPC &copy_from);
   virtual ~NPC();

	enum NPC_attrib {Aggression, Speed};

protected:

   virtual void saveData(pugi::xml_node &entnode) const;
   virtual int loadData(pugi::xml_node &entnode);

   virtual bool setFlagInternal(const char *flagname, bool newval);
   virtual bool isFlagSetInternal(const char *flagname, bool &results);

   void setStartLoc(const char *newloc);

   const char *getStartLoc() const { return _startloc.c_str(); };

   // Adds shared_ptr links between this object and others in the EntityDB. Polymorphic
   virtual void addLinks(EntityDB &edb, std::shared_ptr<Entity> self);

private:

	std::string _startloc;
	
};


#endif
