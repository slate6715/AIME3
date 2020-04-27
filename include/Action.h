#ifndef ACTION_H
#define ACTION_H

#include <chrono>
#include <memory>
#include <bitset>
#include <vector>
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
	
   // Command parse structure. Drives how the string is parsed and sent into the Action
	// object. 
   enum parse_type { 
			Undef,	// Not used default value that will throw an exception
			Single,	// Simplest command - single word
			ActTarg,	// <action> <target>
			ActTargOptCont,// <action> <target> [<preposition> <target2>
			Chat,		// <action> <string>
			Tell		// <action> <target> <string>
		 };

	enum act_types { Hardcoded, Script };

	enum act_flags {
			TargetOrg,	// Target must be an organism (mobile or player)
			TargetLoc,	// Target must be in the current location
			NoLookup		// Action class will not lookup the target object but simply pass in the string
	};

	// Constructors
	Action(const char *id); 
	Action(const Action &copy_from);

   virtual ~Action();

	std::chrono::system_clock::time_point getExecTime() const { return _exec_time; };
	
	void setExecuteNow();
	void setAgent(std::shared_ptr<Organism> agent);
	void setFormat(const char *format);

	int execute(MUD &mud);

	parse_type getParseType() const { return _ptype; };
	const char *getFormat() const { return _format.c_str(); };
	std::shared_ptr<Organism> getAgent() { return _agent; };

	// Gets the parsed input token at the given index
	const char *getToken(unsigned int index) const;

	bool configAction(std::vector<std::string> &tokens, std::string &errmsg);	

protected:

   virtual void saveData(pugi::xml_node &entnode) const;
   virtual int loadData(LogMgr &log, pugi::xml_node &entnode);

private:

   act_types _atype;
	parse_type _ptype;
	std::string _format;

	// When the heartbeat passes this time, the command executes
	std::chrono::system_clock::time_point _exec_time;

	std::vector<std::string> _tokens;

	// For hard-coded actions, pointer to the function
	int (*_act_ptr)(MUD &, Action &);

	// The organism executing this action
	std::shared_ptr<Organism> _agent;

	// Action flags stored here
	std::bitset<32>	_actflags;	
};

struct hardcoded_actions {
   std::string act_id;
   int (*funct_ptr)(MUD &, Action &);
};

#endif
