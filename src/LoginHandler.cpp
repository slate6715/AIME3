#include <exceptions.h>
#include <cstring>
#include <iostream>
#include "LoginHandler.h"
#include "misc.h"
#include "Player.h"
#include "global.h"


LoginHandler::LoginHandler(std::shared_ptr<Player> plr, libconfig::Config &mud_cfg):
								Handler(plr),
								_mud_cfg(mud_cfg),
								_username(""),
								_new_passwd(""),
								_cur_state(AskUser)
{
}

LoginHandler::LoginHandler(const LoginHandler &copy_from):
								Handler(copy_from),
								_mud_cfg(copy_from._mud_cfg),
								_username(copy_from._username),
								_new_passwd(copy_from._new_passwd),
								_cur_state(copy_from._cur_state)
{

}

LoginHandler::~LoginHandler() {

}

/*********************************************************************************************
 * validateUsername - since we're using username to get files, we need to make sure there
 *						    aren't any special characters indicating nefarious actions. Right now
 *							 the code only allows letters and numbers
 *
 *
 *********************************************************************************************/

bool LoginHandler::validateUsername(std::string &name) {
	unsigned short n_id = 0;

	for (unsigned int i=0; i<name.size(); i++) {
		n_id = (unsigned short) name[i];
		if ((n_id < 48) || ((n_id > 57) && (n_id < 65)) || (n_id > 123))
			return false;
	}
	return true;
}

/*********************************************************************************************
 * handleCommand - primary function for the handler. Takes in input from the user and parses
 *					    it. This function handles inputs required to log the user in
 *
 *
 *********************************************************************************************/

int LoginHandler::handleCommand(std::string &cmd) {

	std::string tempname, plrid, userdir;
	int results = 0;

	switch(_cur_state) {
		// The user has entered a username?
		case AskUser:
			lower(cmd);
			if (!validateUsername(cmd)) {
				_plr->sendMsg("Invalid username. Name can only consist of letters or numbers.");
				break;
			}	

			plrid = _plr->getID();
			_mud_cfg.lookupValue("datadir.userdir", userdir);	
			if ((results = _plr->loadUser(userdir.c_str(), cmd.c_str())) == 0) {
				_username = cmd;
				_plr->sendMsg("That user does not exist.\n");
				_cur_state = AskCreate;
				break;
			} else if (results == -1) {
				_plr->sendMsg("Disconnecting.\n");
				_plr->quit();
				break;
			} else {
				_username = cmd;
				_plr->setID(plrid.c_str());
				_cur_state = AskPasswd;
			}
			break;

		case AskCreate:
			lower(cmd);
			if ((cmd.size() == 0) || (std::strncmp(cmd.c_str(), "yes", cmd.length()))) {
				_cur_state = AskUser;
				break;
			}

			_cur_state = CreatePasswd1;
			break;

		case CreatePasswd1:
			_new_passwd = cmd;
			if (_new_passwd.length() < 8) {
				_plr->sendMsg("Password must be at least 8 characters in length.\n");
				break;
			}

			_cur_state = CreatePasswd2;

			break;

		case CreatePasswd2:
			if (cmd.compare(_new_passwd) != 0) {
				_plr->sendMsg("You must enter the same password twice.\n");
				_cur_state = CreatePasswd1;
				break;
			}
			
			// Create the account
			_plr->createPassword(cmd.c_str());

			// Temporarily set the name so we can save the user data--can't permanently change it yet as
			// this will be done when the handler is popped.
			tempname = _plr->getID();
			plrid = "player:" + _username;
			_plr->setID(plrid.c_str());

			_mud_cfg.lookupValue("datadir.userdir", userdir);

			if (!_plr->saveUser(userdir.c_str())) {
				std::string msg("Unable to save user file to ");
				msg += userdir;
				mudlog->writeLog(msg.c_str());
				_plr->sendMsg("Failed saving your user file. Alert an Admin.\n");
				handler_state = Disconnect;
				return 1;
			}
			_plr->setID(tempname.c_str());
			handler_state = Finished;		
			_plr->sendMsg("Successfully created your user!\n");	
			return 1;

		case AskPasswd:
			if (_plr->checkPassword(cmd.c_str())) {
				handler_state = Finished;
				_plr->sendMsg("Successfully logged in!\n");
			   break;					
			}
			_plr->sendMsg("Incorrect password.\n");
	
			break;
		default:
			throw std::runtime_error("Unknown state in LoginHandler::handleCommand. Could not handle.");

			break;
	}
	_plr->sendPrompt();

	return 0;
}


/*********************************************************************************************
 * sendPrompt - send the appropriate prompt to the user for the given handler and state
 *
 *
 *********************************************************************************************/

void LoginHandler::getPrompt(std::string &buf) {
   switch(_cur_state) {
      case AskUser:
			buf = "What shall we call you? ";
         break;

      case AskCreate:
			buf = "Would you like to create that username? (y/n) ";
         break;

      case CreatePasswd1:
			buf = "Enter your new password: ";
         break;

      case CreatePasswd2:
			buf = "Re-enter your new password: ";
         break;

      case AskPasswd:
			buf = "Enter your password: ";
         break;

      default:
         throw std::runtime_error("Unknown state in LoginHandler::handleCommand. Could not handle.");

         break;
   }

}

/*********************************************************************************************
 * prePop - populates the results vector and cleans up, preparing to be popped.
 *
 *		Params:	results - an empty vector to hold results
 *
 *********************************************************************************************/


void LoginHandler::prePop(std::vector<std::string> &results) {
	results.push_back("loggedin");
	results.push_back(_username);
}

/*********************************************************************************************
 * postPush - called right after this handler is created and pushed onto the stack.
 *
 *
 *********************************************************************************************/


void LoginHandler::postPush() {

	sendInfoFiles(_plr, _mud_cfg, "infofiles.welcome");

	_plr->sendPrompt();	
}



