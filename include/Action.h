#ifndef ACTION_H
#define ACTION_H

#include <chrono>
#include <memory>
#include "Entity.h"
#include "Organism.h"
#include "actions.h"

class MUD;

/***************************************************************************************
 * Action - contains all the necessary components to execute a command in the game.
 *				Consists of hard-coded commands and special script-driven commands
 *
 ***************************************************************************************/
class Action : public Entity 
{
public:
	
   // Command parse structure:
	//		Undef - invalid setting, indicates an error somewhere
   //    Single - Just the action, nothing after it
   //    ActSubj - Action followed by a subject (info <filename>)
   //    Tell - action target <long string>
   enum parse_type { Undef, Single, ActSubj, Tell };

	enum act_types { Hardcoded, Script };

	// Constructors
	Action(const char *id, int (*act_ptr)(MUD &, Action &), parse_type ptype);
	Action(const char *id); 
	Action(const Action &copy_from);

   virtual ~Action();

	std::chrono::system_clock::time_point getExecTime() const { return _exec_time; };
	
	void setExecuteNow();
	void setAgent(std::shared_ptr<Organism> agent);

	int execute(MUD &mud);

protected:

   virtual void saveData(pugi::xml_node &entnode) const;
   virtual int loadData(LogMgr &log, pugi::xml_node &entnode);

private:

   act_types _atype;
	parse_type _ptype;

	// When the heartbeat passes this time, the command executes
	std::chrono::system_clock::time_point _exec_time;

	// For hard-coded actions, pointer to the function
	int (*_act_ptr)(MUD &, Action &);

	std::shared_ptr<Organism> _agent;
};


#endif
