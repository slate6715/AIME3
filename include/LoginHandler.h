#ifndef LOGINHANDLER_H
#define LOGINHANDLER_H

#include <string>
#include <libconfig.h++>
#include "Handler.h"
#include "LogMgr.h"

// Handles the entire login process, including account creation
class LoginHandler : public Handler 
{
public:
	LoginHandler(std::shared_ptr<Player> plr, libconfig::Config &mud_cfg, LogMgr &log);
	LoginHandler(const LoginHandler &copy_from);

	virtual ~LoginHandler();

	enum login_state {AskUser, AskCreate, CreatePasswd1, CreatePasswd2, AskPasswd};

	virtual int handleCommand(std::string &cmd);
	virtual void getPrompt(std::string &buf);
   virtual void prePop(std::vector<std::string> &results);
	virtual void postPush();

	bool validateUsername(std::string &name);

private:

	libconfig::Config &_mud_cfg;
	
	LogMgr &_log;

	std::string _username;
	std::string _new_passwd;

	login_state _cur_state;
};


#endif
