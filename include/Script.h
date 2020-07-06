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

   enum script_flags {
         Indefinite,  // Runs this script indefinitely (based on interval) until the script exits with 1
			Placeholder
   };
	
	// Constructors
	Script(const char *id);
	Script(const Script &copy_from);

   virtual ~Script();

   bool addVariable(const char *varname, std::shared_ptr<Physical> variable);
   bool hasVariable(const char *varname);

	void clearVariables();

	float getInterval() const { return _interval; };
	unsigned int getCount() const { return _count; };

	void setInterval(float new_interval) { _interval = new_interval; };
	void setCount(unsigned int new_count) { _count = new_count; };
	
	bool isScriptFlagSet(script_flags flag);

	virtual int execute();

protected:

   virtual void saveData(pugi::xml_node &entnode) const;
   virtual int loadData(pugi::xml_node &entnode);

   virtual bool setFlagInternal(const char *flagname, bool newval);
   virtual bool isFlagSetInternal(const char *flagname, bool &results);

private:

	std::bitset<32> _scriptflags;

	std::vector<std::pair<std::string, std::shared_ptr<Physical>>> _variables;

	float _interval;
	unsigned int _count;

	std::string _code;

	
};

#endif
