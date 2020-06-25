#ifndef SOCIAL_H
#define SOCIAL_H

#include <memory>
#include <bitset>
#include <vector>
#include <map>
#include <string>
#include "Action.h"

struct amplifier {
	amplifier(char in_charmap, std::string in_repl):charmap(in_charmap), repl(in_repl) {};

	char charmap;
	std::string repl;
};

/***************************************************************************************
 * Social - a command that basically just emotes (and maybe triggers a special)
 *
 ***************************************************************************************/
class Social : public Action
{
public:
	
	// Constructors
	Social(const char *id); 
	Social(const Social &copy_from);

   virtual ~Social();

	enum social_flag { Target1Static, Target1Getable, Adverb };

	bool isSocialFlagSet(social_flag stype);

	bool addMessage(const char *msgname, const char *msg);

   virtual bool parseCommand(const char *cmd, std::string &errmsg);
   virtual int execute();
	

protected:

   virtual void saveData(pugi::xml_node &entnode) const;
   virtual int loadData(pugi::xml_node &entnode);

   virtual bool setFlagInternal(const char *flagname, bool newval);
   virtual bool isFlagSetInternal(const char *flagname, bool &results);

private:

	std::map<std::string, std::string> _messages;

	std::map<std::string, amplifier> _amplifiers;

	std::bitset<32> _socialflags;

	std::map<std::string, amplifier>::iterator _selected_amp = _amplifiers.end();
};

#endif
