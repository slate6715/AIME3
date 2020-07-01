#ifndef SCRIPT_H
#define SCRIPT_H

#include <memory>
#include <bitset>
#include <vector>
#include "Entity.h"

class Physical;

/***************************************************************************************
 * Script - an entity set up to run a Python special, usually on a clock timer globally-
 *          executed
 *
 ***************************************************************************************/
class Script : public Entity 
{
public:
	
	// Constructors
	Script(const char *id);
	Script(const Script &copy_from);

   virtual ~Script();

protected:

   virtual void saveData(pugi::xml_node &entnode) const;
   virtual int loadData(pugi::xml_node &entnode);


private:

	unsigned int _repeat;

	std::vector<std::pair<std::string, std::shared_ptr<Physical>>> _variables;

	std::string _script;
};

#endif
