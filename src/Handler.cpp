#include <exceptions.h>
#include <cstring>
#include <iostream>
#include "Handler.h"
#include "strfuncts.h"
#include "Player.h"


/*********************************************************************************************
 * Handler (constructor)
 *	
 *
 *********************************************************************************************/
Handler::Handler(Player &plr):
							handler_state(Active),
							return_val(""),
							_plr(plr)
{
}

// Called by child class
Handler::Handler(const Handler &copy_from):
							handler_state(copy_from.handler_state),
							return_val(copy_from.return_val),
							_plr(copy_from._plr)
{

}


Handler::~Handler() {

}


LoginHandler::LoginHandler(Player &plr, const char *userdir):
								Handler(plr),
								_userdir(userdir),
								_username(""),
								_new_passwd(""),
								_cur_state(AskUser)
{
}

LoginHandler::LoginHandler(const LoginHandler &copy_from):
								Handler(copy_from),
								_userdir(copy_from._userdir),
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

	switch(_cur_state) {
		// The user has entered a username?
		case AskUser:
			lower(cmd);
			if (!validateUsername(cmd)) {
				_plr.sendMsg("Invalid username. Name can only consist of letters or numbers.");
				break;
			}	

			if (!_plr.loadUser(_userdir.c_str(), cmd.c_str())) {
				_username = cmd;
				_plr.sendMsg("That user does not exist.\n");
				_cur_state = AskCreate;
				break;
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
				_plr.sendMsg("Password must be at least 8 characters in length.\n");
				break;
			}

			_cur_state = CreatePasswd2;

			break;

		case CreatePasswd2:
			if (cmd.compare(_new_passwd) != 0) {
				_plr.sendMsg("You must enter the same password twice.\n");
				_cur_state = CreatePasswd1;
				break;
			}
			
			// Create the account
			
			break;

		case AskPasswd:
	
			break;
		default:
			throw std::runtime_error("Unknown state in LoginHandler::handleCommand. Could not handle.");

			break;
	}
	_plr.sendPrompt();

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

GameHandler::GameHandler(Player &plr):
                        Handler(plr)
{
}

GameHandler::GameHandler(const GameHandler &copy_from):
                        Handler(copy_from)
{

}

GameHandler::~GameHandler() {

}

/*********************************************************************************************
 * handleCommand - primary function for the handler. Takes in input from the user and parses
 *                 it. This is the main game handler. 
 *
 *
 *********************************************************************************************/

int GameHandler::handleCommand(std::string &cmd) {

	return 0;
}

/*********************************************************************************************
 * getPrompt - send the appropriate prompt to the user for the given handler and state
 *
 *
 *********************************************************************************************/

void GameHandler::getPrompt(std::string &buf) {
	
}

