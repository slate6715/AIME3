#ifndef LOGINHANDLER_H
#define LOGINHANDLER_H

#include <string>
#include <libconfig.h++>
#include "Handler.h"

// Handles the entire login process, including account creation
class LoginHandler : public Handler 
{
public:
   enum login_state {AskUser, AskPasswd, AskCreate, CreatePasswd1, CreatePasswd2, GetGender, GetRace, GetClass,
                     LoginMenu };

	LoginHandler(std::shared_ptr<Player> plr, libconfig::Config &mud_cfg, login_state start_state=AskUser);
	LoginHandler(const LoginHandler &copy_from);

	virtual ~LoginHandler();

	virtual int handleCommand(std::string &cmd);
	virtual void getPrompt(std::string &buf);
   virtual void prePop(std::vector<std::string> &results);
	virtual void postPush();

	bool validateUsername(std::string &name);

	void sendLoginMenu();

private:

	void addTrait(std::shared_ptr<Player> plr, const char *trait, bool mask = false);

	libconfig::Config &_mud_cfg;
	
	std::string _username;
	std::string _new_passwd;

	login_state _cur_state;
};


#endif
