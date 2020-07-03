#include <iostream>
#include <sstream>
#include "Action.h"
#include "Script.h"
#include "MUD.h"
#include "actions.h"
#include "misc.h"
#include "global.h"


const char *ptype_list[] = {"undef", "single", "acttarg", "acttargcont", "acttargoptcont", "actopttarg", "socialstyle", "chat", "tell", NULL};
const char *aflag_list[] = {"target1mud", "target1loc", "target1inv", "target1org", "target2mud", "target2loc", 
									 "target2inv", "target2org", "nolookup", "aliastarget", "visibleonly", NULL};

hardcoded_actions cmd_array[] = {
		{"infocom", infocom},
		{"gocom", gocom},
		{"lookcom", lookcom},
		{"exitscom", exitscom},
		{"getcom", getcom},
		{"putcom", putcom},
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
		{"equipcom", equipcom},
		{"removecom", removecom},
		{"tiecom", tiecom},
		{"untiecom", untiecom},
		{"failcom", failcom},
		{"gotocom", gotocom},
		{"killcom", killcom},
		{"lightcom", lightcom},
		{"extinguishcom", extinguishcom},
		{"summoncom", summoncom},
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
								_actor(),
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
								_actor(copy_from._actor),
								_actflags(copy_from._actflags),
								_alias(copy_from._alias),
								_pretrig(copy_from._pretrig),
								_posttrig(copy_from._posttrig),
								_target1(copy_from._target1),
								_target2(copy_from._target2)
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
	std::string attstr;

	pugi::xml_attribute attr;

	// These are only mandatory for non-Script entities
	if (dynamic_cast<Script *>(this) != NULL) {
		_atype = ScriptOnly;
		_ptype = Undef;
	} else {
		// Get the acttype - must be either hardcoded or script
		attr = entnode.attribute("acttype");
		if (attr == nullptr) {
			errmsg << "Action '" << getID() << "' missing mandatory acttype field."; 
			mudlog->writeLog(errmsg.str().c_str());
			return 0;
		}

		attstr = attr.value();
		lower(attstr);
		if (attstr.compare("hardcoded") == 0)
			_atype = Hardcoded;
		else if (attstr.compare("scriptonly") == 0)
			_atype = ScriptOnly;
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
	}

   // If it's a hardcoded action, get the mandatory function mapping
	if (_atype == Hardcoded) {
		attr = entnode.attribute("function");
		if (attr == nullptr) {
			errmsg << "Hardcoded Action '" << getID() << "' missing mandatory function field.";
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

int Action::execute() {

	int results = 0;
	if (_atype == Hardcoded) {
		results = (*_act_ptr)(engine, *this);
	}
	else if (_atype == ScriptOnly) {
		// TODO

	} else {
		throw std::runtime_error("Unsupported action type found.");
	}

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
 * setExecute - Sets the execution time to the current time plus the parameter number of seconds
 *
 *		params:	future_secs - decimal value of the number of seconds in the future to place this
 *
 *********************************************************************************************/

void Action::setExecute(float future_secs) {
	_exec_time = std::chrono::system_clock::now() + std::chrono::milliseconds((int) (future_secs * 1000));
}

/*********************************************************************************************
 * setActor - sets the organism that is taking this action
 *
 *    Params:  actor - a shared_ptr that has been dynamically-cast to an Organism pointer (using
 *							  dynamic_pointer_cast to maintain pointer tracking)
 *
 *********************************************************************************************/

void Action::setActor(std::shared_ptr<Organism> actor) {
	_actor = actor;
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
 *					target - target 1 or 2
 *					
 *
 *    Returns: pointer if found/accessible, nullptr otherwise 
 *
 *********************************************************************************************/

std::shared_ptr<Physical> Action::findTarget(const std::string &name, std::string &errmsg, int targetsel)
{
	std::stringstream errmsgstr;

	unsigned int offset = 0;
	if (targetsel == 1) {
		offset = 0;	// Use target1 flags
	} else if (targetsel == 2) {
		offset = 4;	// Adjust flags to target2 variety
	} else {
		throw std::invalid_argument("findTarget called with target not equal to 1 or 2");
	}

	std::shared_ptr<Physical> target;
   std::shared_ptr<Organism> actor = getActor();
   std::shared_ptr<Physical> cur_loc = actor->getCurLoc();

   // Check inventory first 
	if (isActFlagSet((act_flags) (Target1Inv + offset))) {
		target = actor->getContainedByName(name.c_str());
	}

	// Now check location if applicable
	if ((target == nullptr) && (isActFlagSet((act_flags) (Target1Loc + offset)))) {
		target = cur_loc->getContainedByName(name.c_str());
	}

	// Raise an error if we didn't find anything and we're not checking the entire MUD
   if ((target == nullptr) && (!isActFlagSet((act_flags) (Action::Target1MUD + offset)))) {
		// If we don't see a ':', then we're probably targeting a user
		errmsgstr << "You don't see '" << name << "' here.";
		errmsg = errmsgstr.str();
      return NULL;
	}

   // If this needs to target an organism
    if (isActFlagSet((act_flags) (Action::Target1Org + offset))) {
		if ((target != nullptr) && (std::dynamic_pointer_cast<Organism>(target) == nullptr)) {
			errmsgstr << target->getGameName(errmsg) << " is an inanimate object.";
			errmsg = errmsgstr.str();
         return NULL;
		}

		// If we still haven't found it and we're looking for an organism, check the players
		if (target == nullptr) {
			std::string plrid("player:");
			plrid += name;
			if ((target = engine.getUserMgr()->getPlayer(plrid.c_str())) == nullptr) {
				errmsg = "That player does not appear to be available.";
				return NULL;
			}
			
		}
		if (target != nullptr)
			return target;
   }

   // If we got here, then we should check the entire mud
   if ((target == nullptr) && ((target = engine.getEntityDB()->getPhysical(name.c_str())) == nullptr)) {
      errmsg = "That object does not appear to exist.";
      return NULL;

   }

	return target;
 
}


/*********************************************************************************************
 * parseToken - gets the next set of text
 *
 *    Params:	pos - the position in the buffer to start getting the token
 *             buf - buffer containing tokens
 *
 *    Returns: pos at the end of the token parsed
 *
 *********************************************************************************************/

size_t Action::parseToken(size_t pos, std::string &buf) {
	if (buf.size() == 0)
		return 0;

	// Find the next space but skip leading spaces
	size_t start = pos;
	while ((start < buf.size()) && ((pos = buf.find(" ", start)) == 0))
		start++;

	// Oops, all that was left were spaces
	if (start >= buf.size()) {
		pos = buf.size();
		return pos;
	}

	
   if (pos == std::string::npos)
		pos = buf.size();
   addToken(buf.substr(start, pos-start));
   return pos + 1;
}


/*********************************************************************************************
 * parseCommand - parses out the command based on this action's settings and preps to execute
 *
 *    Params:  cmd - the part of the command string after the action name itself
 *					errmsg - populated if an error is encountered
 *
 *    Returns: true if successful, false if an error was found
 *
 *********************************************************************************************/

bool Action::parseCommand(const char *cmd, std::string &errmsg) {
	size_t pos = 0;

	std::string buf = (cmd == NULL) ? "" : cmd;

	// Some commands require the actor to be able to see
	if ((isActFlagSet(VisibleOnly)) && !getActor()->canSee()) {
		errmsg = "You need to be able to see to use that command.\n";
		return false;
	}

	// Lone commands with these parse types are ok, exit early
	if ((getParseType() == Action::Single) || 
		 ((getParseType() == Action::ActOptTarg) && (buf.size() == 0)))
		return true;

   // Get target 1
   if ((getParseType() == Action::ActTargOptCont) ||
       (getParseType() == Action::ActTarg) ||
       (getParseType() == Action::ActTargCont) ||
		 (getParseType() == Action::ActOptTarg)) {

		// If we're here, we should have at least one more token
      if ((buf.size() == 0) && (numTokens() == 0)) {
			errmsg = "Missing target. Format: ";
         errmsg += getFormat();
         return false;
      }

		// Get the rest of the tokens
		while ((pos = parseToken(pos, buf)) < buf.size());
		
      // Get target 1
      if (!isActFlagSet(Action::NoLookup)) {
			// ActOptTarg may have a preposition before target
			unsigned int targ_idx = 0;
			if (getParseType() == Action::ActOptTarg) {
				if (numTokens() >= 2) {
					if (!isPreposition(getToken(1))) {
						errmsg = "Invalid preposition. Format: ";
						errmsg += getFormat();
						return false;
					}
					targ_idx = 1;
				}
			}
			
			std::string targ1 = getToken(targ_idx);
         setTarget1(findTarget(targ1, errmsg, 1));

         if (getTarget1() == nullptr) {
            return false;
         }
      }

   }

   // If we have more words to process
   if ((getParseType() == Action::ActTargOptCont) || (getParseType() == Action::ActTargCont)) {

      if (getParseType() == Action::ActTargCont) {
         if (numTokens() < 2) {
            errmsg = "Missing elements. Format: ";
            errmsg += getFormat();
            return false;
         } else if (numTokens() == 2) {
            if (isPreposition(getToken(1))) {
               errmsg = "Missing target. Format: ";
               errmsg += getFormat();
               return false;
            }

         }
      }

      // Do we need to find target2?
      if (!isActFlagSet(Action::NoLookup) && (numTokens() > 2)) {
         std::string targ2;
         if (numTokens() == 2)
            targ2 = getToken(1);
         else
            targ2 = getToken(2);

         setTarget2(findTarget(targ2, errmsg, 2));
         if (getTarget2() == nullptr) {
            return false;
         }
      }

   }
   else if ((getParseType() == Action::Tell) || (getParseType() == Action::Chat)) {
		if (getParseType() == Action::Tell) {
			// Get the next token - should be the target
			if (buf.size() == 0) {
				errmsg = "Missing elements. Format: ";
				errmsg += getFormat();
				return false;
			}
			pos = parseToken(0, buf);

			// Get target 1
			if (!isActFlagSet(Action::NoLookup)) {
	         setTarget1(findTarget(getToken(0), errmsg, 1));

		      if (getTarget1() == nullptr) {
			      errmsg = "That does not appear to be around.";
				   return false;
				}

			}

		}
		
		if (pos == buf.size()) {
         errmsg = "Invalid format. Should be: ";
         errmsg += getFormat();
         return false;
      }

		while ((pos < buf.size()) && (buf[pos] == ' '))
			pos++;

		if (pos == buf.size()) {
			errmsg = "Silence is golden. Format: ";
			errmsg += getFormat();
			return false;
		}
      addToken(buf.substr(pos, buf.size() - pos+1));
   }
   // If it's not single, then we don't recognize this type
   else if ((getParseType() != Action::ActTarg) && (getParseType() != Action::ActOptTarg)) {
      errmsg = "Unrecognized/unsupported command type for command: ";
      errmsg += getID();
      throw std::runtime_error(errmsg.c_str());
   }
	return true;
}

