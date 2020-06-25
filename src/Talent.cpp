#include <iostream>
#include <sstream>
#include "Talent.h"
#include "MUD.h"
#include "actions.h"
#include "misc.h"
#include "global.h"


const char *tflag_list[] = {"noneyet", NULL};

/*********************************************************************************************
 * Talent (constructor)
 *
 *********************************************************************************************/
Talent::Talent(const char *id):
								Action(id)
{
	_typename = "Talent";

}

// Copy constructor
Talent::Talent(const Talent &copy_from):
								Action(copy_from),
								_talentflags(copy_from._talentflags)
{

}


Talent::~Talent() {

}

/*********************************************************************************************
 * saveData - Called by a child class to save Action-specific data into an XML document
 *
 *    Params:  entnode - This entity's node within the XML tree so attributes can be added to
 *                       it. This should be set up by a child class
 *             log - to log any errors
 *
 *********************************************************************************************/

void Talent::saveData(pugi::xml_node &entnode) const {

	// First, call the parent version
	Talent::saveData(entnode);

   // Add organism data (none yet)
   // pugi::xml_attribute idnode = entnode.append_attribute("id");
   // idnode.set_value(_id.c_str());

}

/*********************************************************************************************
 * loadData - Called by a child class to populate Action-specific data from an XML document
 *
 *    Params:  entnode - This entity's node within the XML tree so attributes can be drawn from
 *                       it
 *             log - to log any errors
 *
 *    Returns: 1 for success, 0 for failure
 *
 *********************************************************************************************/

int Talent::loadData(pugi::xml_node &entnode) {

	// First, call the parent function
	int results = 0;
	if ((results = Action::loadData(entnode)) != 1)
		return results;

/*	std::stringstream errmsg;

	// Get the acttype - must be either hardcoded or script
   pugi::xml_attribute attr = entnode.attribute("acttype");
   if (attr == nullptr) {
		errmsg << "Action '" << getID() << "' missing mandatory acttype field."; 
		mudlog->writeLog(errmsg.str().c_str());
		return 0;
	}

	std::string attstr = attr.value();
	lower(attstr);
	if (attstr.compare("hardcoded") == 0)
		_atype = Hardcoded;
	else if (attstr.compare("script") == 0)
		_atype = Script;
   else if (attstr.compare("trigger") == 0)
      _atype = Trigger;
	else {
		errmsg << "Action '" << getID() << "' acttype field has invalid value: " << attstr;
		mudlog->writeLog(errmsg.str().c_str());
		return 0;
	}

	// Get the parse type
   attr = entnode.attribute("parsetype");
   if (attr == nullptr) {
      errmsg << "Action '" << getID() << "' missing mandatory parsetype field.";
      mudlog->writeLog(errmsg.str().c_str());
      return 0;
   }
	
   attstr = attr.value();
   lower(attstr);

	unsigned int i=0;
	while ((ptype_list[i] != NULL) && (attstr.compare(ptype_list[i]) != 0))
		i++;

   if ((i == 0) || (ptype_list[i] == NULL)) {
      errmsg << "Action '" << getID() << "' parsetype field has invalid value: " << attstr;
      mudlog->writeLog(errmsg.str().c_str());
      return 0;
   }
	_ptype = (parse_type) i;

   // If it's a hardcoded action, get the mandatory function mapping
	if (_atype == Hardcoded) {
		attr = entnode.attribute("function");
		if (attr == nullptr) {
			errmsg << "Hardcoded Action '" << getID() << "' missing mandatory tunction field.";
			mudlog->writeLog(errmsg.str().c_str());
			return 0;
		}

		attstr = attr.value();

		unsigned int i=0;
		while ((cmd_array[i].act_id.size() > 0) && (attstr.compare(cmd_array[i].act_id) != 0))
			i++;

		if (cmd_array[i].funct_ptr == NULL) {
			errmsg << "Hardcoded Action '" << getID() << "' function mapping '" << attstr << "' not found.";
			mudlog->writeLog(errmsg.str().c_str());
			return 0;
		}
		_act_ptr = cmd_array[i].funct_ptr;
	}

   // Get the optional format field
   attr = entnode.attribute("format");
   if (attr != nullptr) {
		setFormat(attr.value());
	}

   // Get the command aliases
   for (pugi::xml_node alias = entnode.child("alias"); alias; alias = alias.next_sibling("alias")) {

      // Get the alternate alias this command can be called with
      pugi::xml_attribute attr = alias.attribute("name");
      if (attr == nullptr) {
         errmsg << "Action '" << getID() << "' Alias node missing mandatory name field.";
         mudlog->writeLog(errmsg.str().c_str());
         return 0;
      }

		std::string aliasstr = attr.value();
		lower(aliasstr);
      _alias.push_back(aliasstr.c_str());

	}	

	attr = entnode.attribute("pretrig");
	if (attr != nullptr)
		_pretrig = attr.value();

   attr = entnode.attribute("posttrig");
   if (attr != nullptr)
      _posttrig = attr.value();
*/
	return 1;
}

/*********************************************************************************************
 * isTalentFlagSet - A faster version of isFlagSet which operates off the enum type for fast
 *						lookup, but only checks Action flags
 *
 *********************************************************************************************/

bool Talent::isTalentFlagSet(talent_flag ttype) {
	return _talentflags[(unsigned int) ttype];
}


/*********************************************************************************************
 * setFlagInternal - given the flag string, first checks the parent for the flag, then checks
 *                   this class' flags
 *
 *
 *********************************************************************************************/

bool Talent::setFlagInternal(const char *flagname, bool newval) {
   if (Action::setFlagInternal(flagname, newval))
      return true;

	std::string flagstr = flagname;
   lower(flagstr);

   size_t i=0;
   while ((tflag_list[i] != NULL) && (flagstr.compare(tflag_list[i]) != 0))
      i++;

   if (tflag_list[i] == NULL)
      return false;

   _talentflags[i] = true;
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

bool Talent::isFlagSetInternal(const char *flagname, bool &results) {
   if (Action::isFlagSetInternal(flagname, results))
      return true;

   std::string flagstr = flagname;
   lower(flagstr);

   size_t i=0;
   while ((tflag_list[i] != NULL) && (flagstr.compare(tflag_list[i]) != 0))
      i++;

   if (tflag_list[i] == NULL)
      return false;

	results =_talentflags[i];
   return true;

}

