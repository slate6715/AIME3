#ifndef TALENT_H
#define TALENT_H

#include <chrono>
#include <memory>
#include <bitset>
#include <vector>
#include "Action.h"


/***************************************************************************************
 * Talent - an executable spell, skill, or other talent that can be assigned to an organism
 *				and executed
 *
 ***************************************************************************************/
class Talent : public Action
{
public:
	
	// Constructors
	Talent(const char *id); 
	Talent(const Talent &copy_from);

   virtual ~Talent();

	enum talent_flag { NoneYet };

	bool isTalentFlagSet(talent_flag ttype);

protected:

   virtual void saveData(pugi::xml_node &entnode) const;
   virtual int loadData(pugi::xml_node &entnode);

   virtual bool setFlagInternal(const char *flagname, bool newval);
   virtual bool isFlagSetInternal(const char *flagname, bool &results);

private:

	std::bitset<32> _talentflags;
};

#endif
