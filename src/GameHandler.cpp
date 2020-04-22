#include <exceptions.h>
#include <cstring>
#include <iostream>
#include "GameHandler.h"
#include "strfuncts.h"
#include "Player.h"


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

