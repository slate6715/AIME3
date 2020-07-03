#ifndef SCRIPTT_H
#define SCRIPTT_H

#include <memory>
#include <bitset>
#include <vector>
#include "Action.h"

/***************************************************************************************
 * Script - an entity set up to run a Python special, usually on a clock timer globally-
 *          executed
 *
 ***************************************************************************************/
class Script : public Action
{
public:
	
	// Constructors
	Script(const char *id);
	Script(const Script &copy_from);

   virtual ~Script();

   bool addVariable(const char *varname, std::shared_ptr<Physical> variable);
   bool hasVariable(const char *varname);

	void clearVariables();

	float getInterval() const { return _interval; };
	unsigned int getCount() const { return _count; };

	virtual int execute();

protected:

   virtual void saveData(pugi::xml_node &entnode) const;
   virtual int loadData(pugi::xml_node &entnode);

private:

	std::vector<std::pair<std::string, std::shared_ptr<Physical>>> _variables;

	float _interval;
	unsigned int _count;

	std::string _code;

	
};

#endif
