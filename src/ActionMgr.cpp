#include <libconfig.h++>
#include <iostream>
#include <chrono>
#include <boost/filesystem.hpp>
#include "ActionMgr.h"
#include "misc.h"
#include "global.h"

namespace lc = libconfig;

/*********************************************************************************************
 * ActionMgr (constructor) - 
 *
 *********************************************************************************************/
ActionMgr::ActionMgr():
					_action_db(),
					_action_queue()
{


}


ActionMgr::ActionMgr(const ActionMgr &copy_from):
					_action_db(copy_from._action_db),
					_action_queue(copy_from._action_queue)
{

}


ActionMgr::~ActionMgr() {

}

/*********************************************************************************************
 * initialize - pulls config settings required for this class from the config file into the
 *				    object and create hard-coded actions
 *
 *    Params:  cfg_info - The libconfig::Config object that stores all the necessary config info
 *                        to set up the user manager 
 *
 *		Returns: number of actions loaded
 *
 *********************************************************************************************/

unsigned int ActionMgr::initialize(lc::Config &cfg_info) {
	
	// Load the actions from the Actions directory
	std::string actiondir;
	cfg_info.lookupValue("datadir.actiondir", actiondir);

   // Initialize the fast lookup iterators
   for (unsigned int i=0; i<26; i++)
      _abbrev_table[i] = _action_db.end();

	// Load our actions from the actiondir files	
	unsigned int count = loadActions(actiondir.c_str());

	return count;
}

/*********************************************************************************************
 * loadActions - reads in the given actions directory and loads all those files
 *
 *    Params:  actiondir - path to the actions directory where the actions files are
 *
 *		REturns: the number read in
 *
 *********************************************************************************************/


unsigned int ActionMgr::loadActions(const char *actiondir) {
   std::vector<std::string> files;
   boost::filesystem::path p(actiondir);
  
   if (!boost::filesystem::exists(actiondir)) {
      std::string msg("Actiondir defined in config file doesn't appear to exist at: ");
      msg += actiondir;
      throw std::runtime_error(msg.c_str());
   }
   boost::filesystem::directory_iterator start(p), end;
   std::transform(start, end, std::back_inserter(files), path_leaf_string());

	unsigned int count = 0;
   pugi::xml_document actionfile;
   for (unsigned int i=0; i<files.size(); i++) {
      std::cout << "Action file: " << files[i] << std::endl;
      std::string filepath(actiondir);
      filepath += "/";
      filepath += files[i].c_str();

      pugi::xml_parse_result result = actionfile.load_file(filepath.c_str());

      if (!result) {
         std::string msg("Unable to open/parse Action file: ");
         msg += filepath;
			msg += ", Error: ";
			msg += result.description();
         mudlog->writeLog(msg.c_str());
         continue;
      }

		// Loop through all the actions in this file
      for (pugi::xml_node actnode = actionfile.child("action"); actnode; actnode = actnode.next_sibling("action")) {

			Action *new_act = new Action("temp");
			if (!new_act->loadEntity(actnode)) {
				std::string msg("Corrupted action file for: ");
				msg += new_act->getID();
				mudlog->writeLog(msg);
				delete new_act;
				continue;
			}

			auto newact_it = _action_db.insert(std::pair<std::string, std::shared_ptr<Action>>(new_act->getID(),
                                                   std::shared_ptr<Action>(new_act)));

			// Add alias entries for this action
			std::vector<std::string> aliases = new_act->getAliases();
			for (unsigned int j=0; j<aliases.size(); j++) {
				std::string full_id("action@");
				full_id += aliases[j];
				
				_action_db.insert(std::pair<std::string, std::shared_ptr<Action>>(
										full_id.c_str(), newact_it.first->second));

			}
			count++;
		}
	}

	// Now generate an abbreviation lookup lexical table
	auto action_it = _action_db.begin();
	char last_ltr = '.';
	std::string id;

	// Loop through all the actions
	for (; action_it != _action_db.end(); action_it++) {
		action_it->second->getNameID(id);

		// If this is the start of a new first letter, assign a lookup iterator
		if (id[0] != last_ltr) {
			unsigned int idx = (unsigned int) id[0] - (unsigned int) 'a';
			_abbrev_table[idx] = action_it;
			last_ltr = id[0];
		}
	}	

	return count;
}


/*********************************************************************************************
 * handleActions - goes through the action queue, executing those actions whose timer is < now().
 *				This function basically handles the dyanmics of the game. All entities that are
 *				"doing something" are doing it in this function using an action in the queue
 *
 *    Params:  cfg_info - The libconfig::Config object that stores all the necessary config info
 *                        to set up the user manager
 *
 *********************************************************************************************/

void ActionMgr::handleActions(MUD &engine) {

	auto cur_time = std::chrono::system_clock::now();

	auto aptr = _action_queue.begin();
	while ((aptr != _action_queue.end()) && ((*aptr)->getExecTime() <= cur_time)) {
		(*aptr)->execute(engine);
		aptr = _action_queue.erase(aptr);
	}

}

/*********************************************************************************************
 * preAction - player submits a command string and it is parsed with some error checking inside
 *				   the action itself. If incorrect, errmsg is populated. If the command appears
 *			      good, then a pointer to the Action to be executed is returned so it can be
 *					populated with any remaining details.
 *
 *    Params:  cmd - the string provided by the player
 *					errmsg - buffer that gets populated with error information if there is an issue 
 *
 *		Returns: NULL if it failed, a point to a cloned Action object if successful. This object
 *             should be returned using the execAction() function or deleted to avoid mem leaks
 *
 *********************************************************************************************/

Action *ActionMgr::preAction(const char *cmd, std::string &errmsg) {
	std::string buf = cmd;

	// Odd situation where nothing was submitted
	if (buf.size() == 0)
	{
		errmsg = "No command was provided.";
		return NULL;
	}

	// Start building the list of elements for this parse
	std::vector<std::string> elements;
	
	size_t pos = buf.find(" ");

	// No space in this command, it's just a single word
	if (pos == std::string::npos) {
		elements.push_back(buf);
	} 
	// Else grab the first word so we can look it up
	else {
		elements.push_back(buf.substr(0, pos));
	}

	lower(elements[0]);
	std::shared_ptr<Action> aptr = findAction(elements[0].c_str());

	// Not found, return NULL
	if (aptr == nullptr) {
		errmsg = "I do not understand the command '";
		errmsg += elements[0];
		errmsg += "'";
		return NULL;
	}

	size_t start = pos+1;
	// We expect format <action> <subject> [<from> <container>]
	if ((aptr->getParseType() == Action::ActTargOptCont) || (aptr->getParseType() == Action::ActTarg)) {
		while ((pos = buf.find(" ", start)) != std::string::npos) {
			elements.push_back(buf.substr(start, pos-start));
			start = pos + 1;
		}

		// Get the last element
		if (start < buf.size()) {
			elements.push_back(buf.substr(start, buf.size() - start));
		}

		if (aptr->getParseType() == Action::ActTarg) {
			if ((elements.size() == 1) && (!aptr->isActFlagSet(Action::AliasTarget))) {
				errmsg = "Missing target. Format: ";
				errmsg += aptr->getFormat();
				return NULL;
			}
		}

	} else if (aptr->getParseType() == Action::Look) {
		// First get the next token. If none exist, then we're done
		if (start < buf.size()) {
			if ((pos = buf.find(" ")) == std::string::npos)
				pos = buf.size();

			std::string token = buf.substr(start, pos-start);
			start = pos + 1;

			// Check for a preposition
			lower(token);
			if ((token.compare("at") == 0) || (token.compare("in") == 0)) {
				elements.push_back(token);

				// Prepositions need to be followed by a target
				if (start >= buf.size()) {
					errmsg = "Missing target. Format: ";
					errmsg += aptr->getFormat();
					return NULL;
				}

				if ((pos = buf.find(" ")) == std::string::npos)
					pos = buf.size();
				token = buf.substr(start, pos-start);
			} 
			// Now get the target
			elements.push_back(token);
		}
	}
	else if ((aptr->getParseType() == Action::Tell) || (aptr->getParseType() == Action::Chat)) {
	
		// Get the target if it's a tell
		if (aptr->getParseType() == Action::Tell) {
			if ((pos = buf.find(" ", start)) == std::string::npos) {
				errmsg = "Invalid format. Should be: ";
				errmsg += aptr->getFormat();
				return NULL;
			}
			elements.push_back(buf.substr(start, pos - start));
			start = pos + 1;
		}

		// Get the string
		if (start >= buf.size()) {
         errmsg = "Invalid format. Should be: ";
         errmsg += aptr->getFormat();
         return NULL;
		}

		elements.push_back(buf.substr(start, buf.size() - start));
	}
	// If it's not single, then we don't recognize this type 
	else if (aptr->getParseType() != Action::Single) {
		errmsg = "Unrecognized command type for command: ";
		errmsg += aptr->getID();
		throw std::runtime_error(errmsg.c_str()); 
	}

	Action *new_act = new Action(*aptr);

	// Action does some custom error-checking on the parse structure and preps for execution
	if (!new_act->configAction(elements, errmsg)) {
		delete new_act;
		return NULL;
	}
	// Command line actions are executed right away
	new_act->setExecuteNow();

	// _action_queue.insert(std::shared_ptr<Action>(new_act));

	return new_act;
}

/*********************************************************************************************
 * cloneAction - similar to preAction, except it's meant for mobiles or specials. The cmd must
 *					  be exactly the ID of the action and no parsing happens. The action is cloned
 *               and returned so it can be populated with details for execution.
 *
 *    Params:  cmd - the string provided by the player
 *
 *    Returns: NULL if it failed, a point to a cloned Action object if successful. This object
 *             should be returned using the execAction() function or deleted to avoid mem leaks
 *
 *********************************************************************************************/

Action *ActionMgr::cloneAction(const char *cmd) {
	auto cmd_it = _action_db.find(cmd);

	if (cmd_it == _action_db.end())
		return NULL;

	return new Action(*(cmd_it->second));
}

/*********************************************************************************************
 * execAction - A cloned action is passed into the ActionMgr to be placed on the execution
 *					 queue for eventual execution
 *
 *    Params:  exec_act - A populated action 
 *             errmsg - buffer that gets populated with error information if there is an issue
 *
 *
 *********************************************************************************************/

void ActionMgr::execAction(Action *exec_act) {
	_action_queue.insert(std::shared_ptr<Action>(exec_act));
}

/*********************************************************************************************
 * findAction - takes a string command and first does a literal search on it. If not found,
 *				    then does a slower iterative search assuming this was an abbreviation
 *
 *    Params:  cmd - the string provided by the player
 *             errmsg - buffer that gets populated with error information if there is an issue
 *
 *    Returns: NULL if it failed, a point to a cloned Action object if successful. This object
 *             should be returned using the execAction() function or deleted to avoid mem leaks
 *
 *********************************************************************************************/

std::shared_ptr<Action> ActionMgr::findAction(const char *cmd) {
	std::string cmdstr(cmd);

   std::string fullname("action@");
   fullname += cmdstr;

   // Look for it using a literal search
   auto mapptr = _action_db.find(fullname);

	// Found it, return!
   if (mapptr != _action_db.end()) {
		return mapptr->second;
   }

   // If not found using a literal search, use an abbreviated search
	unsigned int idx = (unsigned int) cmdstr[0] - (unsigned int) 'a';

	// If there's no index'd commands, then no commands start with this letter
	if (_abbrev_table[idx] == _action_db.end())
		return nullptr;

	// Else we found indexed commands, start searching
	auto a_iter = _abbrev_table[idx];
	std::string buf;
	while (a_iter != _action_db.end()) {
		a_iter->second->getNameID(buf);

		// If we moved into different letters
		if (buf[0] != cmdstr[0])
			return nullptr;

		// We have a match!
		if (cmdstr.compare(0, cmdstr.size(), buf))
			return a_iter->second;
	}
	return nullptr;
}
