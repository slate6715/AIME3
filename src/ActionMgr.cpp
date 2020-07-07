#include <libconfig.h++>
#include <iostream>
#include <chrono>
#include <boost/filesystem.hpp>
#include <memory>
#include <regex>
#include "ActionMgr.h"
#include "misc.h"
#include "global.h"
#include "Talent.h"
#include "Social.h"
#include "Player.h"
#include "Script.h"

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
	std::string actiondir, talentsdir, socialsdir;
	cfg_info.lookupValue("datadir.actiondir", actiondir);
	cfg_info.lookupValue("datadir.talentsdir", talentsdir);
	cfg_info.lookupValue("datadir.socialsdir", socialsdir);

   // Initialize the fast lookup iterators
   for (unsigned int i=0; i<26; i++)
      _abbrev_table[i] = _action_db.end();

	// Load our actions from the actiondir files	
	unsigned int count = loadActions(actiondir.c_str());

	// If the talent directory is different from our actions directory, load talents
	if (talentsdir.compare(actiondir) != 0) 
		count += loadActions(talentsdir.c_str());

	// Same with socials
	if ((socialsdir.compare(actiondir) != 0) && (socialsdir.compare(talentsdir) != 0))
		count += loadActions(socialsdir.c_str());

	return count;
}


/*********************************************************************************************
 * add - adds an action to the database after first searching for duplicates
 *
 *    Params:  actiondir - path to the actions directory where the actions files are
 *
 *    REturns: the number read in
 *
 *********************************************************************************************/


bool ActionMgr::add(Action *new_act) {
	std::stringstream errmsg;
	std::string namebuf;

	// make sure it doesn't already exist first
	auto newact_it = _action_db.find(new_act->getNameID(namebuf));
	if (newact_it != _action_db.end()) {
		errmsg << "Action " << new_act->getID() << " already exists in " << newact_it->second->getID() <<
							" as either its name or alias.";
		mudlog->writeLog(errmsg.str().c_str());
		return false;
	}

	auto inserted = _action_db.insert(std::pair<std::string, std::shared_ptr<Action>>(namebuf,
                                                   std::shared_ptr<Action>(new_act)));

   // Add alias entries for this action
	std::vector<std::string> aliases = new_act->getAliases();
   for (unsigned int j=0; j<aliases.size(); j++) {

		if ((newact_it = _action_db.find(aliases[j])) != _action_db.end()) {
			errmsg << "Action " << new_act->getID() << " alias " << aliases[j] << " already exists in " << 
							newact_it->second->getID() << " as either its name or alias. Not added.";
			mudlog->writeLog(errmsg.str().c_str());
			return false;

		}
			
      _action_db.insert(std::pair<std::string, std::shared_ptr<Action>>(aliases[j], inserted.first->second));
   }
	return true;
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

			if (!add(new_act))
				delete new_act;
			else
				count++;
		}
      // Loop through all the talents in this file
      for (pugi::xml_node talnode = actionfile.child("talent"); talnode; talnode = talnode.next_sibling("talent")) {

         Talent *new_tal = new Talent("temp");
         if (!new_tal->loadEntity(talnode)) {
            std::string msg("Corrupted talent file for: ");
            msg += new_tal->getID();
            mudlog->writeLog(msg);
            delete new_tal;
            continue;
         }


         if (!add(new_tal))
				delete new_tal;
			else
            count++;
      }
      // Loop through all the socials in this file
      for (pugi::xml_node socnode = actionfile.child("social"); socnode; socnode = socnode.next_sibling("social")) {

         Social *new_soc = new Social("temp");
         if (!new_soc->loadEntity(socnode)) {
            std::string msg("Corrupted social file for: ");
            msg += new_soc->getID();
            mudlog->writeLog(msg);
            delete new_soc;
            continue;
         }


         if (!add(new_soc))
				delete new_soc;
			else
            count++;
      }

	}

	// Now generate an abbreviation lookup lexical table
	auto action_it = _action_db.begin();
	char last_ltr = '.';
	std::string name;

	// Loop through all the actions
	for (; action_it != _action_db.end(); action_it++) {
		name = action_it->first;

		// If this is the start of a new first letter, assign a lookup iterator
		if (name[0] != last_ltr) {
			unsigned int idx = (unsigned int) name[0] - (unsigned int) 'a';
			_abbrev_table[idx] = action_it;
			last_ltr = name[0];
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

void ActionMgr::handleActions() {

	auto cur_time = std::chrono::system_clock::now();

	// Loop through all actions in the queue, executing them
	auto aptr = _action_queue.begin();
	while ((aptr != _action_queue.end()) && ((*aptr)->getExecTime() <= cur_time)) {


		int results = aptr->get()->execute();

		// Check for post-action triggers if the command was successful
		std::string posttrig = aptr->get()->getPostTrig();
		if ((results > 0) && (posttrig.size() > 0)) {
			handleSpecials(aptr->get(), posttrig.c_str());
		}

		// std::shared_ptr<Organism> actor = aptr->get()->getActor();
		//if (actor != nullptr)
			//actor->sendPrompt();

		// The action may repeat itself at an interval (like Scripts do)
		if (results == 2) {
			std::shared_ptr<Action> cont_ptr = *aptr;
			aptr = _action_queue.erase(aptr);
			_action_queue.insert(cont_ptr);
		} else {
			aptr = _action_queue.erase(aptr);
		}
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

Action *ActionMgr::preAction(const char *cmd, std::string &errmsg, 
																	std::shared_ptr<Organism> actor) {
	std::string buf = cmd;
	
	// Odd situation where nothing was submitted
	if (buf.size() == 0)
	{
		errmsg = "No command was provided.";
		return NULL;
	}

	size_t pos = buf.find(" ");

	std::string cmdstr;
	// No space in this command, it's just a single word
	if (pos == std::string::npos) {
		pos = buf.size();
		cmdstr = cmd;
		buf.clear();
	} 
	// Else grab the first word so we can look it up
	else {
		cmdstr = buf.substr(0, pos);
		buf.erase(0, pos);
	}

	lower(cmdstr);
	std::shared_ptr<Action> actptr = findAction(cmdstr.c_str());

	// Not found, return NULL
	if (actptr == nullptr) {
		errmsg = "I do not understand the command '";
		errmsg += cmdstr;
		errmsg += "'";
		return NULL;
	}


	Action *new_act;
	std::shared_ptr<Social> sptr = std::dynamic_pointer_cast<Social>(actptr);
	std::shared_ptr<Talent> tptr = std::dynamic_pointer_cast<Talent>(actptr);

	if (sptr != nullptr)
		new_act = new Social(*sptr);
	else if (tptr != nullptr)
		new_act = new Talent(*tptr);
	else
		new_act = new Action(*actptr);

	new_act->setActor(actor);

	// if this is an aliastarget like using north instead of "go north", add the command as first token
	if ((buf.size() == 0) && new_act->isActFlagSet(Action::AliasTarget))
		new_act->addToken(cmdstr);

	// Now parse the command
	if (!new_act->parseCommand(buf.c_str(), errmsg)) {
		delete new_act;
		return NULL;
	}

	// Check for pretrig specials attached to targets or the current location
	std::string pretrig = new_act->getPreTrig();
	if (pretrig.size() > 0) {
		int results = handleSpecials(new_act, pretrig.c_str());
		if (results == 2) {
			delete new_act;
			return NULL;
		}
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

   std::regex cmdcheck("[a-z]+");
   lower(cmdstr);

   if (!std::regex_match(cmdstr, cmdcheck)) {
      return nullptr;
   }

   // Look for it using a literal search
   auto mapptr = _action_db.find(cmdstr);

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
	std::string fullid, name;
	while (a_iter != _action_db.end()) {
		fullid = a_iter->first;
		size_t pos = fullid.find(":");
		name = fullid.substr(pos+1, fullid.size() - pos);

		// If we moved into different letters
		if (name[0] != cmdstr[0])
			return nullptr;

		// No idea why the shortened version of std::string::compare wasn't working here... 
		pos = 0;
		while ((pos < cmdstr.size()) && (pos < name.size()) && (cmdstr[pos] == name[pos]))
			pos++;
	
		if (pos == cmdstr.size())
			return a_iter->second;

		a_iter++;
	}
	return nullptr;
}

/*********************************************************************************************
 * execSpecial - looks for the given trigger attached to this entity and executes it if found.
 *
 *    Params:  trigger - the trigger string to match
 *             actor - the organism executing this action
 *
 *    Returns: -1 for error
 *             0 indicates the trigger was not found attached to this entity
 *             1 indicates the trigger executed and the command process should proceed normally
 *             2 indicates the trigger executed and the command process should terminate
 *
 *********************************************************************************************/

int ActionMgr::handleSpecials(Action *action, const char *trigger) {
	int results = 0;

	std::vector<std::pair<std::string, std::shared_ptr<Physical>>> variables;

	if (action->getActor() != nullptr)
		variables.push_back(std::pair<std::string, std::shared_ptr<Physical>>("actor", action->getActor()));
   if (action->getTarget1() != nullptr)
      variables.push_back(std::pair<std::string, std::shared_ptr<Physical>>("target1", action->getTarget1()));
   if (action->getTarget2() != nullptr)
      variables.push_back(std::pair<std::string, std::shared_ptr<Physical>>("target2", action->getTarget2()));


   // Look for specials on target1 if applicable
   if (action->getTarget1() != nullptr) {
      // If the special ran and said to terminate, don't continue
      if ((results = action->getTarget1()->execSpecial(trigger, variables)) == 2) {
         return 2;
      }
   }

   // Look for specials on target2
   if (action->getTarget2() != nullptr) {
      // If the special ran and said to terminate, don't continue
      if ((results = action->getTarget2()->execSpecial(trigger, variables)) == 2) {
         return 2;
      }
   }

   // Look for specials in the location
   if ((results = action->getActor()->getCurLoc()->execSpecial(trigger, variables)) == 2) {
      return 2;
   }

	return results;
}


/*********************************************************************************************
 * addScript - adds a script entity to the queue to be executed
 *
 *
 *********************************************************************************************/

bool ActionMgr::addScript(std::shared_ptr<Script> new_script) {
	// Set the execute time to now plus interval
	new_script->setExecute(new_script->getInterval());

   _action_queue.insert(new_script);	
	return true;
}


