#include <exceptions.h>
#include <cstring>
#include <iostream>
#include "GameHandler.h"
#include "misc.h"
#include "Player.h"
#include "ActionMgr.h"


GameHandler::GameHandler(Player &plr, ActionMgr &actions):
                        Handler(plr),
								_actions(actions)
{
}

GameHandler::GameHandler(const GameHandler &copy_from):
                        Handler(copy_from),
								_actions(copy_from._actions)
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

	std::string buf("Received command: ");
	buf += cmd;
	buf += "\n";
	
	_plr.sendMsg(buf);

	_plr.sendPrompt();

	return 0;
}

/*********************************************************************************************
 * getPrompt - send the appropriate prompt to the user for the given handler and state
 *
 *
 *********************************************************************************************/

void GameHandler::getPrompt(std::string &buf) {
	buf = "TempPrompt> ";	
}

/*********************************************************************************************
 * prePop - populates the results vector and cleans up, preparing to be popped.
 *
 *    Params:  results - an empty vector to hold results
 *
 *********************************************************************************************/


void GameHandler::prePop(std::vector<std::string> &results) {
	throw std::runtime_error("This should not be called.");
}


/*********************************************************************************************
 * postPush - called right after this handler is created and pushed onto the stack.
 *
 *
 *********************************************************************************************/


void GameHandler::postPush() {
}
