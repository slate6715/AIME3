#ifndef GAMEHANDLER_H
#define GAMEHANDLER_H

#include <string>
#include "Handler.h"


// The primary input handler--should never be popped off the stack
class GameHandler : public Handler
{
public:
	GameHandler(Player &plr);
	GameHandler(const GameHandler &copy_from);

	virtual ~GameHandler();

	virtual int handleCommand(std::string &cmd);
	virtual void getPrompt(std::string &buf);
   virtual void prePop(std::vector<std::string> &results);

private:
		
};

#endif
