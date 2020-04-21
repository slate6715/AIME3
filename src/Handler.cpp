#include <exceptions.h>
#include "Handler.h"


/*********************************************************************************************
 * Handler (constructor)
 *	
 *
 *********************************************************************************************/
Handler::Handler()
{
}

// Called by child class
Handler::Handler(const Handler &copy_from)
{

}


Handler::~Handler() {

}


LoginHandler::LoginHandler():
								Handler(),
								_username(""),
								_new_passwd(""),
								_cur_state(AskUser)
{
}

LoginHandler::LoginHandler(const LoginHandler &copy_from):
								Handler(copy_from),
								_username(copy_from._username),
								_new_passwd(copy_from._new_passwd),
								_cur_state(copy_from._cur_state)
{

}

LoginHandler::~LoginHandler() {

}

/*********************************************************************************************
 * handleCommand - primary function for the handler. Takes in input from the user and parses
 *					    it. This function handles inputs required to log the user in
 *
 *
 *********************************************************************************************/

int LoginHandler::handleCommand(const char *cmd) {

	switch(_cur_state) {
		case AskUser:

			break;
		case AskCreate:
			break;

		case CreatePasswd1:
	
			break;

		case CreatePasswd2:

			break;

		case AskPasswd:
	
			break;
		default:
			throw std::runtime_error("Unknown state in LoginHandler::handleCommand. Could not handle.");

			break;
	}
	return 0;
}


/*********************************************************************************************
 * sendPrompt - send the appropriate prompt to the user for the given handler and state
 *
 *
 *********************************************************************************************/

void LoginHandler::sendPrompt() {

}

GameHandler::GameHandler():
                        Handler()
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

int GameHandler::handleCommand(const char *cmd) {

	return 0;
}

/*********************************************************************************************
 * sendPrompt - send the appropriate prompt to the user for the given handler and state
 *
 *
 *********************************************************************************************/

void GameHandler::sendPrompt() {

}

