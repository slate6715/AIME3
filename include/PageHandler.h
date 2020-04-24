#ifndef PAGEHANDLER_H
#define PAGEHANDLER_H

#include <string>
#include "Handler.h"
#include "LogMgr.h"

// Short-term handler for displaying long documents. Code should load the handler with text, then 
// push onto the user stack 
class PageHandler : public Handler
{
public:
	PageHandler(Player &plr, LogMgr &log, unsigned int lines_per_page);
	PageHandler(const PageHandler &copy_from);

	virtual ~PageHandler();

	virtual int handleCommand(std::string &cmd);
	virtual void getPrompt(std::string &buf);
   virtual void prePop(std::vector<std::string> &results);
	virtual void postPush();

	void addContent(const char *msg);
	void addContent(std::string &msg);
	void addFileContent(const char *filename);

private:
	bool showNextPage();

	LogMgr &_log;

	size_t _lines_per_page;
	std::string _to_display;
	
};

#endif
