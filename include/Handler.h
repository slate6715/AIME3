#ifndef HANDLER_H
#define HANDLER_H

#include <string>
#include <vector>

class Player;

/***************************************************************************************
 * Handler - Parses the user input and executes commands differently depending on the
 *				 handler actually used. 
 *
 ***************************************************************************************/

// Generic class, should not be enstantiated
class Handler
{
public:
	
	Handler(Player &plr);
	Handler(const Handler &copy_from);

   virtual ~Handler();

	virtual int handleCommand(std::string &cmd) = 0;
	virtual void getPrompt(std::string &buf) = 0;
	virtual void prePop(std::vector<std::string> &results) = 0;
	virtual void postPush() = 0;

	// The handler state - finished = read to pop
	enum hstate_types { Active, Finished, Disconnect };
	hstate_types handler_state;

	// When the handler is complete or in a non-Active state, this provides more details
	std::string return_val;

protected:
	Player &_plr;

private:

};

#endif
