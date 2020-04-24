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
	ActionMgr(LogMgr &mud_log);
   ActionMgr(const ActionMgr &copy_from);
   virtual ~ActionMgr();

	// Initialize certain variables for this class from the config file
	void initialize(libconfig::Config &cfg_info);

	// Go through the action queue, executing those whose timer is < now()
	void handleActions();

private:

	// The log manager passed in by reference on object creation
	LogMgr &_mud_log;

	// The database of available actions and aliases 
	std::map<std::string, std::shared_ptr<Action>> _action_db;

	std::multiset<std::shared_ptr<Action>, compare_msa> _action_queue;

};


#endif
