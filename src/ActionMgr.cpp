#include <libconfig.h++>
#include <iostream>
#include <chrono>
#include <boost/filesystem.hpp>
#include "ActionMgr.h"
#include "misc.h"

namespace lc = libconfig;

hardcoded_actions cmd_array[] = {
   {"info", infocom, Action::ActSubj, ""},
   {"",0, Action::Undef, ""}
};

/*********************************************************************************************
 * ActionMgr (constructor) - 
 *
 *********************************************************************************************/
ActionMgr::ActionMgr(LogMgr &mud_log):
					_mud_log(mud_log),
					_action_db(),
					_action_queue()
{


}


ActionMgr::ActionMgr(const ActionMgr &copy_from):
					_mud_log(copy_from._mud_log),
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
 *********************************************************************************************/

void ActionMgr::initialize(lc::Config &cfg_info) {
	
	unsigned int i = 0;
	
	Action *aptr = NULL;

	// Loop through the command list, creating hard-coded actions
	while (cmd_array[i].act_id.size() > 0) {
		aptr = new Action(cmd_array[i].act_id.c_str(), cmd_array[i].funct_ptr, cmd_array[i].ptype);

		// Set up aliases here (TODO)


		// Add this new action to the database of available commands
		_action_db.insert(std::pair<std::string, std::shared_ptr<Action>>(aptr->getID(), 
																	std::shared_ptr<Action>(aptr)));
		std::cout << "Added action: " << aptr->getID() << std::endl;
		i++;
	}

	// Load the script actions from the Actions directory
	std::string actiondir;
	cfg_info.lookupValue("datadir.actiondir", actiondir);
	std::vector<std::string> files;
	boost::filesystem::path p(actiondir);
	if (!boost::filesystem::exists(p)) {
		std::string msg("Actiondir defined in config file doesn't appear to exist at: ");
		msg += actiondir;
		throw std::runtime_error(msg.c_str());
	}
	boost::filesystem::directory_iterator start(p), end;
	std::transform(start, end, std::back_inserter(files), path_leaf_string());	


	for (unsigned int i=0; i<files.size(); i++) {
		std::cout << "Action file: " << files[i] << std::endl;
		// TODO - add code to load the files
	}

	
	// Build our abbrev_table for fast abbreviation lookups

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

	std::cout << "Looking up command: " << elements[0] << std::endl;

	lower(elements[0]);

	auto aptr = _action_db.find(elements[0]);

	// Find it using a literal search
	if (aptr != _action_db.end()) {
		return new Action(*(aptr->second));
	}

	// If not found using a literal search, use an abbreviated search

	// Not found, return NULL
	errmsg = "I do not understand the command '";
	errmsg += elements[0];
	errmsg += "'";
	return NULL;		
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


