#ifndef GAMEHANDLER_H
#define GAMEHANDLER_H

#include <string>
#include "Handler.h"
#include "ActionMgr.h"

// The primary input handler--should never be popped off the stack
class GameHandler : public Handler
{
public:
	GameHandler(Player &plr, ActionMgr &actions);
	GameHandler(const GameHandler &copy_from);

	virtual ~GameHandler();

	virtual int handleCommand(std::string &cmd);
	virtual void getPrompt(std::string &buf);
   virtual void prePop(std::vector<std::string> &results);
	virtual void postPush();

private:

	ActionMgr &_actions;
	
};

#endif
