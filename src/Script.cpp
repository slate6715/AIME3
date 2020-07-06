#include <iostream>
#include <sstream>
#include "global.h"
#include "misc.h"
#include "Script.h"

const char *scrflag_list[] = {"indefinite", "placeholder", NULL};

Script::Script(const char *id):
								Action(id),
								_interval(0),
								_count(0),
								_code("")
{
	_typename = "Script";
}

Script::Script(const Script &copy_from):
										Action(copy_from),
										_interval(copy_from._interval),
										_count(copy_from._count),
										_code(copy_from._code)
{

}

Script::~Script() {

}

/*********************************************************************************************
 * saveData - Called by a child class to save Script-specific data into an XML document
 *
 *    Params:  entnode - This entity's node within the XML tree so attributes can be added to
 *                       it. This should be set up by a child class
 *             log - to log any errors
 *
 *********************************************************************************************/

void Script::saveData(pugi::xml_node &entnode) const {

   // First, call the parent version
   Action::saveData(entnode);

   // Add organism data (none yet)
   // pugi::xml_attribute idnode = entnode.append_attribute("id");
   // idnode.set_value(_id.c_str());

}

/*********************************************************************************************
 * loadData - Called by a child class to populate Script-specific data from an XML document
 *
 *    Params:  entnode - This entity's node within the XML tree so attributes can be drawn from
 *                       it
 *             log - to log any errors
 *
 *    Returns: 1 for success, 0 for failure
 *
 *********************************************************************************************/

int Script::loadData(pugi::xml_node &entnode) {
   // First, call the parent function
   int results = 0;
   if ((results = Action::loadData(entnode)) != 1)
      return results;

   std::stringstream errmsg;
	std::string buf;

   // Get the interval in seconds between running this script
   pugi::xml_attribute attr = entnode.attribute("interval");
   if (attr != nullptr) {
		buf = attr.value();
		_interval = std::stof(buf);
   }

   // Get the number of times this should execute
   attr = entnode.attribute("count");
   if (attr != nullptr) {
      buf = attr.value();
      _count = (unsigned int) std::stoul(buf);
   }

	// Code section that contains the script
	pugi::xml_node code = entnode.child("code");
	if (code == nullptr) {
      errmsg << "Script '" << getID() << "' missing mandatory code node.";
      mudlog->writeLog(errmsg.str().c_str());
      return 0;
	}
	_code = code.child_value();

	return 1;
}

/*********************************************************************************************
 * addVariable - adds the specified variable to the list so they will be available when the script
 *               is executed.
 *
 *    Params:  varname - the string the script can use to refer to this variable
 *					variable - pointer to the Physical object for the script
 *
 *    Returns: true for success, false if it was already loaded
 *
 *********************************************************************************************/

bool Script::addVariable(const char *varname, std::shared_ptr<Physical> variable) {
	if (hasVariable(varname))
		return false;

	_variables.push_back(std::pair<std::string, std::shared_ptr<Physical>>(varname, variable));
	return true;
}

/*********************************************************************************************
 * hasVariable - returns true if the variable is in the list by name, false otherwise
 *
 *    Returns: true for success, false if it was already loaded
 *
 *********************************************************************************************/

bool Script::hasVariable(const char *varname) {
	for (unsigned int i=0; i<_variables.size(); i++) {
		if (_variables[i].first.compare(varname) == 0)
			return true;
	}
	return false;
}

/*********************************************************************************************
 * clearVariable - clears the variable list
 *
 *********************************************************************************************/

void Script::clearVariables() {
	_variables.clear();
}



/*********************************************************************************************
 * execute - runs this script
 *
 *		Returns:	1 if done and should be removed from the execution queue
 *					2 if done and should be re-added to the queue at the usual interval
 *
 *********************************************************************************************/

int Script::execute() {
	ScriptEngine &se = *engine.getScriptEngine();

	// Set up the variables
	for (unsigned int i=0; i<_variables.size(); i++) {
		se.setVariable(_variables[i].first.c_str(), _variables[i].second);
	}

	if (!isScriptFlagSet(Indefinite))
		_count--;

	se.setVariableConst("count", (int) _count);
	se.setVariableConst("interval", _interval);

	// If the script exited with code 1, don't execute this script anymore
	if (se.execute(_code.c_str()) == 1)
		_count = 0;

	// Needs to execute a few more times
	if (_count > 0) {
		setExecute(_interval);	// Set interval to the future
		return 2;
	}

	// Done executing, purge
	return 1;
}

/*********************************************************************************************
 * isActFlagSet - A faster version of isFlagSet which operates off the enum type for fast
 *                lookup, but only checks Action flags
 *
 *********************************************************************************************/

bool Script::isScriptFlagSet(script_flags stype) {
   return _scriptflags[(unsigned int) stype];
}


/*********************************************************************************************
 * setFlagInternal - given the flag string, first checks the parent for the flag, then checks
 *                   this class' flags
 *
 *
 *********************************************************************************************/

bool Script::setFlagInternal(const char *flagname, bool newval) {
   if (Action::setFlagInternal(flagname, newval))
      return true;

   std::string flagstr = flagname;
   lower(flagstr);

   size_t i=0;
   while ((scrflag_list[i] != NULL) && (flagstr.compare(scrflag_list[i]) != 0))
      i++;

   if (scrflag_list[i] == NULL)
      return false;

   _scriptflags[i] = true;
   return true;
}

/*********************************************************************************************
 * isFlagSetInternal - given the flag string, first checks the parent for the flag, then checks
 *                   this class' flags
 *
 *    Params:  flagname - flag to set
 *             results - if found, what the flag is set to
 *
 *    Returns: true if the flag was found, false otherwise
 *
 *********************************************************************************************/

bool Script::isFlagSetInternal(const char *flagname, bool &results) {
   if (Action::isFlagSetInternal(flagname, results))
      return true;

   std::string flagstr = flagname;
   lower(flagstr);

   size_t i=0;
   while ((scrflag_list[i] != NULL) && (flagstr.compare(scrflag_list[i]) != 0))
      i++;

   if (scrflag_list[i] == NULL)
      return false;

   results =_scriptflags[i];
   return true;

}

