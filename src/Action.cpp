#include <iostream>
#include <sstream>
#include "Action.h"
#include "MUD.h"
#include "actions.h"
#include "misc.h"
#include "global.h"


const char *ptype_list[] = {"undef", "single", "acttarg", "acttargoptcont", "look", "chat", "tell", NULL};
const char *aflag_list[] = {"targetmud", "targetloc", "targetinv", "targetorg", "nolookup", "aliastarget", 
									NULL};

hardcoded_actions cmd_array[] = {
		{"infocom", infocom},
		{"gocom", gocom},
		{"lookcom", lookcom},
		{"exitscom", exitscom},
		{"getcom", getcom},
		{"dropcom", dropcom},
		{"inventorycom", inventorycom},
		{"userscom", userscom},
		{"saycom", saycom},
		{"chatcom", chatcom},
		{"tellcom", tellcom},
		{"quitcom", quitcom},
      {"opencom", opencom},
      {"closecom", closecom},
      {"statscom", statscom},
		{"",0}
};


/*********************************************************************************************
 * Action (constructor) - Called by a child class to initialize any Action elements. Used to
 *								  create a scripted version of an Action
 *
 *********************************************************************************************/
Action::Action(const char *id):
								Entity(id),
								_atype(Hardcoded),
								_ptype(Undef),
								_format(""),
								_exec_time(),
								_tokens(),
								_act_ptr(NULL),
								_agent(),
								_actflags()
{
	_typename = "Action";

}

// Copy constructor
Action::Action(const Action &copy_from):
								Entity(copy_from),
								_atype(copy_from._atype),
								_ptype(copy_from._ptype),
								_format(copy_from._format),
								_exec_time(copy_from._exec_time),
								_tokens(copy_from._tokens),
								_act_ptr(copy_from._act_ptr),
								_agent(copy_from._agent),
								_actflags(copy_from._actflags)
{

}


Action::~Action() {

}

/*********************************************************************************************
 * saveData - Called by a child class to save Action-specific data into an XML document
 *
 *    Params:  entnode - This entity's node within the XML tree so attributes can be added to
 *                       it. This should be set up by a child class
 *             log - to log any errors
 *
 *********************************************************************************************/

void Action::saveData(pugi::xml_node &entnode) const {

	// First, call the parent version
	Entity::saveData(entnode);

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

int Action::loadData(pugi::xml_node &entnode) {

	// First, call the parent function
	int results = 0;
	if ((results = Entity::loadData(entnode)) != 1)
		return results;

	std::stringstream errmsg;

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

   // Get the Exits (if any)
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

	return 1;
}

/*********************************************************************************************
 * execute - executes this action
 *
 *    Params:  
 *
 *    Returns: 1 for success, 0 for failure
 *
 *********************************************************************************************/

int Action::execute(MUD &engine) {

	int results = 0;
	if (_atype == Hardcoded) {
		results = (*_act_ptr)(engine, *this);
	}
	else if (_atype == Script) {
		// TODO

	} else {
		throw std::runtime_error("Unsupported action type found.");
	}

	_agent->sendPrompt();

	return results;
}

/*********************************************************************************************
 * setExecuteNow - Sets the execution time to the current time
 *
 *********************************************************************************************/

void Action::setExecuteNow()
{
	_exec_time = std::chrono::system_clock::now();
}

/*********************************************************************************************
 * setAgent - sets the organism that is taking this action
 *
 *    Params:  agent - a shared_ptr that has been dynamically-cast to an Organism pointer (using
 *							  dynamic_pointer_cast to maintain pointer tracking)
 *
 *********************************************************************************************/

void Action::setAgent(std::shared_ptr<Organism> agent) {
	_agent = agent;
}

/*********************************************************************************************
 * getToken - gets the user input token at the given index, which was populated based on the
 *				  parse_type of this action. 
 *
 *    Params:  index - the array index of the requested token
 *
 *********************************************************************************************/

const char *Action::getToken(unsigned int index) const {
	if (index >= _tokens.size())
		throw std::invalid_argument("Action getToken index out of bounds.");

	return _tokens[index].c_str();
}


/*********************************************************************************************
 * setFormat - Sets the string that defines the command's format. Is usually sent to the player
 *				   when they type it incorrectly.
 *
 *********************************************************************************************/

void Action::setFormat(const char *format) {
	_format = format;
}

/*********************************************************************************************
 * isActFlagSet - A faster version of isFlagSet which operates off the enum type for fast
 *						lookup, but only checks Action flags
 *
 *********************************************************************************************/

bool Action::isActFlagSet(act_flags atype) {
	return _actflags[(unsigned int) atype];
}


/*********************************************************************************************
 * configAction - Prepares this action to be executed. Does some specific error checking on
 *                the action. 
 *		Params:	tokens - parsed user input of strings in vector format
 *					errmsg - to be populated if an error is found
 *
 *		Returns:	true if no errors, false otherwise
 *
 *********************************************************************************************/

bool Action::configAction(std::vector<std::string> &tokens, std::string &errmsg) {
	_tokens = tokens;

	// Prob more to come here
	(void) errmsg;

	return true;	
}

/*********************************************************************************************
 * setFlagInternal - given the flag string, first checks the parent for the flag, then checks
 *                   this class' flags
 *
 *
 *********************************************************************************************/

bool Action::setFlagInternal(const char *flagname, bool newval) {
   if (Entity::setFlagInternal(flagname, newval))
      return true;

	std::string flagstr = flagname;
   lower(flagstr);

   size_t i=0;
   while ((aflag_list[i] != NULL) && (flagstr.compare(aflag_list[i]) != 0))
      i++;

   if (aflag_list[i] == NULL)
      return false;

   _actflags[i] = true;
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

bool Action::isFlagSetInternal(const char *flagname, bool &results) {
   if (Entity::isFlagSetInternal(flagname, results))
      return true;

   std::string flagstr = flagname;
   lower(flagstr);

   size_t i=0;
   while ((aflag_list[i] != NULL) && (flagstr.compare(aflag_list[i]) != 0))
      i++;

   if (aflag_list[i] == NULL)
      return false;

	results =_actflags[i];
   return true;

}


/*********************************************************************************************
 * findTarget - Finds a target based on flags, location, etc--basic availability and populates
 * errors in the errmsg string
 *
 *    Params:  name - the object name to look for
 *             errmsg - populates an error message for the user
 *
 *    Returns: pointer if found/accessible, nullptr otherwise 
 *
 *********************************************************************************************/

std::shared_ptr<Entity> Action::findTarget(std::string &name, std::string &errmsg, EntityDB &edb, 
																			UserMgr &umgr) {

	std::shared_ptr<Entity> target1;
   std::shared_ptr<Organism> agent = getAgent();
   std::shared_ptr<Entity> cur_loc = agent->getCurLoc();

   // Check inventory first 
	if (isActFlagSet(TargetInv)) {
		target1 = agent->getContained(name.c_str());
	}

	// Now check location if applicable
	if ((target1 == nullptr) && (isActFlagSet(TargetLoc))) {
		target1 = cur_loc->getContained(name.c_str());
	}

	// Raise an error if we didn't find anything and we're not checking the entire MUD
   if ((target1 == nullptr) && (!isActFlagSet(Action::TargetMUD))) {
		// If we don't see a ':', then we're probably targeting a user
		errmsg = "That does not appear to be here.";
      return NULL;
	}

   // If this needs to target an organism
    if (isActFlagSet(Action::TargetOrg)) {
		if ((target1 != nullptr) && (std::dynamic_pointer_cast<Organism>(target1) == nullptr)) {
			errmsg = "That is an inanimate object.";
         return NULL;
		}

		// If we still haven't found it and we're looking for an organism, check the players
		if (target1 == nullptr) {
			std::string plrid("player:");
			plrid += name;
			if ((target1 = umgr.getPlayer(plrid.c_str())) == nullptr) {
				errmsg = "That player does not appear to be available.";
				return NULL;
			}
			
		}
   }
   // If we still haven't found it, check mud-wide (must be in form zone:obj)
   else if ((target1 == nullptr) && 
				((target1 = edb.getEntity(name.c_str())) == nullptr)) {
		errmsg = "That object does not appear to exist.";
      return NULL;
   }
	return target1;
 
}

