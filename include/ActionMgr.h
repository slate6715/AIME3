#ifndef ACTIONMGR_H
#define ACTIONMGR_H

#include <map>
#include <memory>
#include <thread>
#include <libconfig.h++>
#include <set>
#include "Action.h"

class Player;
class Script;

struct compare_msa {
	bool operator()(const std::shared_ptr<Action> &lhs,
						 const std::shared_ptr<Action> &rhs) const {
		return lhs->getExecTime() > rhs->getExecTime();
	}
};

/****************************************************************************************
 * ActionMgr - class that stores Actions (commands) and facilitates execution. All actions,
 *			 whether movement by mobiles, fighting or users examining objects are placed into
 *			 the execution queue, which is addressed each heartbeat. 
 *
 ****************************************************************************************/
class ActionMgr 
{
public:
	ActionMgr();
   ActionMgr(const ActionMgr &copy_from);
   virtual ~ActionMgr();

	// Initialize certain variables for this class from the config file
	unsigned int initialize(libconfig::Config &cfg_info);

	// Load the different groups of actions
	unsigned int loadActions(const char *actiondir);

	// Go through the action queue, executing those whose timer is < now()
	void handleActions();

	Action *preAction(const char *cmd, std::string &errmsg, 
														std::shared_ptr<Organism> actor);
	Action *cloneAction(const char *cmd);
	void execAction(Action *exec_act);
	
	std::shared_ptr<Action> findAction(const char *cmd);

	// Adds a script to the queue to be executed	
	bool addScript(std::shared_ptr<Script> new_script);

private:
	int handleSpecials(Action *action, const char *trigger);

	bool add(Action *new_act);

	// The database of available actions and aliases 
	std::map<std::string, std::shared_ptr<Action>> _action_db;

	// One for each letter--points to the first action of that letter for quick lookup on
	// abbreviated actions
	std::map<std::string, std::shared_ptr<Action>>::iterator _abbrev_table[26];

	std::multiset<std::shared_ptr<Action>, compare_msa> _action_queue;

};


#endif
