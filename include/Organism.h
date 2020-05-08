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

	// Virtual functions that do nothing for NPCs
   virtual bool sendFile(const char *filename) { (void) filename; return true; };
   // Send a message to this entity or its contents - class-specific behavior
   virtual void sendMsg(const char *msg, std::shared_ptr<Entity> exclude=nullptr)                                                                                      { (void) msg; (void) exclude; };
   virtual void sendMsg(std::string &msg, std::shared_ptr<Entity> exclude=nullptr)                                                                                     { (void) msg; (void) exclude; };
	virtual void sendPrompt() {};
	virtual void sendCurLocation() {};
	virtual void sendExits() {};

	virtual const char *getDesc() { return _desc.c_str(); };

protected:
	Organism(const char *id);	// Must be called from the child constructor
	Organism(const Organism &copy_from);

   virtual void saveData(pugi::xml_node &entnode) const;
   virtual int loadData(pugi::xml_node &entnode);

   virtual bool setFlagInternal(const char *flagname, bool newval);
   virtual bool isFlagSetInternal(const char *flagname, bool &results);

private:
	std::string _desc;

	
};


#endif
