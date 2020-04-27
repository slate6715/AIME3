#include <iostream>
#include <sstream>
#include "Action.h"
#include "MUD.h"
#include "actions.h"
#include "misc.h"


const char *ptype_list[] = {"undef", "single", "acttarg", "acttargoptcont", "chat", "tell", NULL};
const char *aflag_list[] = {"targetorg", "targetloc", "nolookup", NULL};

hardcoded_actions cmd_array[] = {
		{"infocom", infocom},
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

int Action::loadData(LogMgr &log, pugi::xml_node &entnode) {

	// First, call the parent function
	int results = 0;
	if ((results = Entity::loadData(log, entnode)) != 1)
		return results;

	std::stringstream errmsg;

	// Get the acttype - must be either hardcoded or script
   pugi::xml_attribute attr = entnode.attribute("ActType");
   if (attr == nullptr) {
		errmsg << "Action '" << getID() << "' missing mandatory ActType field."; 
		log.writeLog(errmsg.str().c_str());
		return 0;
	}

	std::string attstr = attr.value();
	lower(attstr);
	if (attstr.compare("hardcoded") == 0)
		_atype = Hardcoded;
	else if (attstr.compare("script") == 0)
		_atype = Script;
	else {
		errmsg << "Action '" << getID() << "' ActType field has invalid value: " << attstr;
		log.writeLog(errmsg.str().c_str());
		return 0;
	}

	// Get the parse type
   attr = entnode.attribute("ParseType");
   if (attr == nullptr) {
      errmsg << "Action '" << getID() << "' missing mandatory ParseType field.";
      log.writeLog(errmsg.str().c_str());
      return 0;
   }
	
   attstr = attr.value();
   lower(attstr);

	unsigned int i=0;
	while ((ptype_list[i] != NULL) && (attstr.compare(ptype_list[i]) != 0))
		i++;

   if ((i == 0) || (ptype_list[i] == NULL)) {
      errmsg << "Action '" << getID() << "' ParseType field has invalid value: " << attstr;
      log.writeLog(errmsg.str().c_str());
      return 0;
   }
	_ptype = (parse_type) i;

   // If it's a hardcoded action, get the mandatory function mapping
	if (_atype == Hardcoded) {
		attr = entnode.attribute("Function");
		if (attr == nullptr) {
			errmsg << "Hardcoded Action '" << getID() << "' missing mandatory Function field.";
			log.writeLog(errmsg.str().c_str());
			return 0;
		}

		attstr = attr.value();

		unsigned int i=0;
		while ((cmd_array[i].act_id.size() > 0) && (attstr.compare(cmd_array[i].act_id) != 0))
			i++;

		if (cmd_array[i].funct_ptr == NULL) {
			errmsg << "Hardcoded Action '" << getID() << "' function mapping '" << attstr << "' not found.";
			log.writeLog(errmsg.str().c_str());
			return 0;
		}
		_act_ptr = cmd_array[i].funct_ptr;
	}

   // Get the optional format field
   attr = entnode.attribute("Format");
   if (attr != nullptr) {
		setFormat(attr.value());
	}

	// Get the Action Flags (if any)
	for (pugi::xml_node flag = entnode.child("ActFlag"); flag; flag = flag.next_sibling("ActFlag")) {
		std::string flagstr = flag.child_value();
		lower(flagstr);
	
		i=0;
		while ((aflag_list[i] != NULL) && (flagstr.compare(aflag_list[i]) != 0))
			i++;

		if (ptype_list[i] == NULL)
		{
			errmsg << "Action '" << getID() << "' ActFlag '" << flagstr << "' is not recognized.";
			log.writeLog(errmsg.str().c_str());
			return 0;
		}

		_actflags[i] = true;
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
	std::cout << "Executing: " << getID() << std::endl;

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

	return true;	
}

