#ifndef HANDLER_H
#define HANDLER_H


/***************************************************************************************
 * Handler - Parses the user input and executes commands differently depending on the
 *				 handler actually used. 
 *
 ***************************************************************************************/
class Handler
{
public:
	
	Handler();
	Handler(const Handler &copy_from);

   virtual ~Handler();

	virtual int handleCommand(const char *cmd) = 0;
	virtual int sendPrompt() = 0;

protected:

private:

};

class LoginHandler : public Handler 
{
public:
	LoginHandler();
	LoginHandler(const LoginHandler &copy_from);

	virtual ~LoginHandler();

	enum login_state {AskUser, AskCreate, CreatePasswd1, CreatePasswd2, AskPasswd};

	virtual int handleCommand(const char *cmd);
	virtual void sendPrompt();

private:
	std::string _username;
	std::string _new_passwd;

	login_state _cur_state;
};

class GameHandler : public Handler
{
public:
	GameHandler();
	GameHandler(const GameHandler &copy_from);

	virtual ~GameHandler();

	virtual int handleCommand(const char *cmd);
	virtual void sendPrompt();

private:
		
};

#endif
