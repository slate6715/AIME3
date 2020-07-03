#ifndef ACTION_H
#define ACTION_H

#include <chrono>
#include <memory>
#include <bitset>
#include <vector>
#include "Entity.h"
#include "actions.h"

class MUD;
class UserMgr;
class Organism;
class EntityDB;

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
			ActTargCont,	// <action> <target> <preposition> <target2>
			ActTargOptCont,// <action> <target> [<preposition> <target2>
			ActOptTarg,		// <action> [[<preposition>] <target>]
			SocialStyle,	// <action> [<target>] [<amplifier>]
			Chat,		// <action> <string>
			Tell		// <action> <target> <string>
		 };

	enum act_types { Hardcoded, ScriptOnly, Trigger };

	enum act_flags {
			Target1MUD,	 // Target1 might be in the broader MUD (not in current loc)
			Target1Loc,	 // Look for target1 in the actor's current location
			Target1Inv,	 // Look in the actor's inventory for the target
			Target1Org,	 // Target1 must be an organism (mobile or player)
         Target2MUD,   // Target2 might be in the broader MUD (not in current loc)
         Target2Loc,   // Look for target2 in the actor's current location
         Target2Inv,   // Look in the actor's inventory for the target2
         Target2Org,   // Target2 must be an organism (mobile or player)
			NoLookup,	 // Action class will not lookup the target object but simply pass in the string
			AliasTarget, // Aliases can act as the target (such as with "go east" and "east")
			VisibleOnly  // Command only works if the actor can see in the room 
	};

	// Constructors
	Action(const char *id); 
	Action(const Action &copy_from);

   virtual ~Action();

	std::chrono::system_clock::time_point getExecTime() const { return _exec_time; };
	
	// Set this to execute right away
	void setExecuteNow();

	// Set this to execute x seconds in the future
	void setExecute(float future_secs);

	void setActor(std::shared_ptr<Organism> actor);
	void setFormat(const char *format);
	void setTarget1(std::shared_ptr<Physical> target) { _target1 = target; };
   void setTarget2(std::shared_ptr<Physical> target) { _target2 = target; };

	virtual int execute();

	parse_type getParseType() const { return _ptype; };
	const char *getFormat() const { return _format.c_str(); };
	std::shared_ptr<Organism> getActor() { return _actor; };
	bool isActFlagSet(act_flags atype);

	virtual bool parseCommand(const char *cmd, std::string &errmsg);

	// Gets the parsed input token at the given index
	const char *getToken(unsigned int index) const;
	unsigned int numTokens() const { return _tokens.size(); };
	void addToken(const char *str) { _tokens.push_back(str); };
	void addToken(std::string str) { _tokens.push_back(str); };

	// Copies the alias list into a new vector
	std::vector<std::string> getAliases() { return _alias; };

	const char *getPreTrig() const { return _pretrig.c_str(); };
	const char *getPostTrig() const { return _posttrig.c_str(); };

	// Finds a target based on flags, location, etc--basic availability and populates
	// errors in the errmsg string
	std::shared_ptr<Physical> findTarget(const std::string &name, std::string &errmsg, int targetsel = 1);

	std::shared_ptr<Physical> getTarget1() { return _target1; };
	std::shared_ptr<Physical> getTarget2() { return _target2; };

protected:

   virtual void saveData(pugi::xml_node &entnode) const;
   virtual int loadData(pugi::xml_node &entnode);

   virtual bool setFlagInternal(const char *flagname, bool newval);
   virtual bool isFlagSetInternal(const char *flagname, bool &results);

	size_t parseToken(size_t pos, std::string &buf);

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
	std::shared_ptr<Organism> _actor;

	// Action flags stored here
	std::bitset<32>	_actflags;	

	// Alias - alternate names for this command
	std::vector<std::string> _alias;

	// Specials triggers for this action - pre happens before the command is called
	// and post after the command finishes executing
	std::string _pretrig;
	std::string _posttrig;

	// Target pointers for action execution
	std::shared_ptr<Physical> _target1;

	std::shared_ptr<Physical> _target2;
};

struct hardcoded_actions {
   std::string act_id;
   int (*funct_ptr)(MUD &, Action &);
};

#endif
