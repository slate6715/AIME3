#ifndef ACTIONMGR_H
#define ACTIONMGR_H

#include <map>
#include <memory>
#include <thread>
#include <libconfig.h++>
#include <set>
#include "Player.h"
#include "Action.h"

// Set up a compare function for the multiset
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

	unsigned int loadActions(const char *actiondir);

	// Go through the action queue, executing those whose timer is < now()
	void handleActions(MUD &engine);

	Action *preAction(const char *cmd, std::string &errmsg, 
														std::shared_ptr<Organism> actor);
	Action *cloneAction(const char *cmd);
	void execAction(Action *exec_act);
	
	std::shared_ptr<Action> findAction(const char *cmd);


private:
	int handleSpecials(Action *action, const char *trigger);

	// The database of available actions and aliases 
	std::map<std::string, std::shared_ptr<Action>> _action_db;

	// One for each letter--points to the first action of that letter for quick lookup on
	// abbreviated actions
	std::map<std::string, std::shared_ptr<Action>>::iterator _abbrev_table[26];

	std::multiset<std::shared_ptr<Action>, compare_msa> _action_queue;

};


#endif
