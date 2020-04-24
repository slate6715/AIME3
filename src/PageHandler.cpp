#include <exceptions.h>
#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>
#include "PageHandler.h"
#include "misc.h"
#include "Player.h"


PageHandler::PageHandler(Player &plr, LogMgr &log, unsigned int lines_per_page):
                        Handler(plr),
								_log(log),
								_lines_per_page(lines_per_page),
								_to_display("")
{
}

PageHandler::PageHandler(const PageHandler &copy_from):
                        Handler(copy_from),
								_log(copy_from._log),
								_lines_per_page(copy_from._lines_per_page),
								_to_display(copy_from._to_display)
{

}

PageHandler::~PageHandler() {

}

/*********************************************************************************************
 * handleCommand - primary function for the handler. Takes in input from the user and parses
 *                 it. This is the main game handler. 
 *
 *
 *********************************************************************************************/

int PageHandler::handleCommand(std::string &cmd) {
	// Simpily check for a C to cancel, otherwise keep going
	lower(cmd);

	if (((cmd.size() > 0) && (cmd[0] == 'c')) || showNextPage()) 
	{
		handler_state = Finished;
		return 1;
	}

	return 0;
}

/*********************************************************************************************
 * getPrompt - send the appropriate prompt to the user for the given handler and state
 *
 *
 *********************************************************************************************/

void PageHandler::getPrompt(std::string &buf) {
	buf = "Press enter to display another page, 'C' to Cancel> ";	
}

/*********************************************************************************************
 * postPush - Executes code when this handler is pushed on the stack
 *
 *    Params:  results - an empty vector to hold results
 *
 *********************************************************************************************/


void PageHandler::postPush() {
   // Nothing to do here.

}

/*********************************************************************************************
 * prePop - populates the results vector and cleans up, preparing to be popped.
 *
 *    Params:  results - an empty vector to hold results
 *
 *********************************************************************************************/


void PageHandler::prePop(std::vector<std::string> &results) {
	// Nothing to do here.
	results.push_back("none");

}

/*********************************************************************************************
 * addContent - Adds text to the handler to be displayed
 *
 *    Params:  msg - the string to add to what will be displayed
 *
 *********************************************************************************************/

void PageHandler::addContent(const char *msg) {
	_to_display += msg;
}

void PageHandler::addContent(std::string &msg){
	_to_display += msg;
}


/*********************************************************************************************
 * addFileContent - Takes in a file path, opens the file, reads the content and adds it to the total
 *
 *    Params:  filename - path of the file to open
 *
 *********************************************************************************************/

void PageHandler::addFileContent(const char *filename) {
   std::ifstream readfile;

   readfile.exceptions(std::ifstream::failbit | std::ifstream::badbit);


   try {
      readfile.open(filename, std::ifstream::out);

      // Allocate string memory space for the whole file
      std::string buf;
      readfile.seekg(0, std::ios::end);
      buf.reserve((long unsigned int) readfile.tellg());
      readfile.seekg(0, std::ios::beg);

      // Read in the file all at once
      buf.assign((std::istreambuf_iterator<char>(readfile)), std::istreambuf_iterator<char>());
      _plr.sendMsg(buf);

      readfile.close();
		_to_display += buf;
   }
   catch (std::ifstream::failure &e) {
      std::stringstream msg;

      msg << "Attempted to open/send file '" << filename << "' to player '" << _plr.getID() << "' failed. Error: " << e.what();
      _log.writeLog(msg.str().c_str());
   }

}

/*********************************************************************************************
 * showNextPage - displays the next set of lines
 *
 *		Returns:		true if that was the last of it, false otherwise
 *
 *********************************************************************************************/

bool PageHandler::showNextPage() {
	size_t count = 0;
	size_t pos = 0;
	while (((pos = _to_display.find("\n", pos+1))  != std::string::npos) && 
			  (count < _lines_per_page))
		count++;

	std::string sendbuf;
	if (pos == std::string::npos) {
		sendbuf = _to_display;
		_plr.sendMsg(sendbuf);
		return true;
	}

	sendbuf = _to_display.substr(0, pos+1);
	_to_display.erase(0, pos+1);
	_plr.sendMsg(sendbuf);
	_plr.sendPrompt();
	return false;
}
