#ifndef HANDLER_H
#define HANDLER_H

#include <string>

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

	// The handler state - finished = read to pop
	enum hstate_types { Active, Finished };
	hstate_types handler_state;

	// When the handler is complete or in a non-Active state, this provides more details
	std::string return_val;

protected:
	Player &_plr;

private:

};

// Handles the entire login process, including account creation
class LoginHandler : public Handler 
{
public:
	LoginHandler(Player &plr, const char *userdir);
	LoginHandler(const LoginHandler &copy_from);

	virtual ~LoginHandler();

	enum login_state {AskUser, AskCreate, CreatePasswd1, CreatePasswd2, AskPasswd};

	virtual int handleCommand(std::string &cmd);
	virtual void getPrompt(std::string &buf);

	bool validateUsername(std::string &name);

private:

	std::string _userdir;

	std::string _username;
	std::string _new_passwd;

	login_state _cur_state;
};

// The primary input handler--should never be popped off the stack
class GameHandler : public Handler
{
public:
	GameHandler(Player &plr);
	GameHandler(const GameHandler &copy_from);

	virtual ~GameHandler();

	virtual int handleCommand(std::string &cmd);
	virtual void getPrompt(std::string &buf);

private:
		
};

#endif
