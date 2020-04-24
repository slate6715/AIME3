#ifndef ACTION_H
#define ACTION_H

#include <chrono>
#include "Entity.h"

/***************************************************************************************
 * Action - contains all the necessary components to execute a command in the game.
 *				Consists of hard-coded commands and special script-driven commands
 *
 ***************************************************************************************/
class Action : public Entity 
{
public:
	
	Action(const char *id);
	Action(const Action &copy_from);

   virtual ~Action();

	std::chrono::system_clock::time_point getExecTime() const { return _exec_time; };

protected:

   virtual void saveData(pugi::xml_node &entnode) const;
   virtual int loadData(LogMgr &log, pugi::xml_node &entnode);

private:
	// When the heartbeat passes this time, the command executes
	std::chrono::system_clock::time_point _exec_time;
};


#endif
