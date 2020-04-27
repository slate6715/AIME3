#include <iostream>
#include <libconfig.h++>
#include <regex>
#include "actions.h"
#include "MUD.h"
#include "Action.h"
#include "misc.h"


/*******************************************************************************************
 * clrNewlines - removes \r and \n from the string passed into buf
 *******************************************************************************************/
int infocom(MUD &engine, Action &act_used) {

	std::shared_ptr<Organism> agent = act_used.getAgent();


	libconfig::Config &mud_cfg = *(engine.getConfig());

	// First, check to make sure the request uses only letters and/or numbers
	std::regex fncheck("[a-z0-9]+");
	std::string infoname = act_used.getToken(1);
	lower(infoname);
	
	if (!std::regex_match(infoname, fncheck)) {
		agent->sendMsg("The info filename you are seeking can only consist of letters and numbers.\n");
		return 0;		
	}

	std::string fullpath;
	mud_cfg.lookupValue("datadir.infodir", fullpath);

	fullpath += "/";
	fullpath += act_used.getToken(1);
	fullpath += ".info";

	if (!agent->sendFile(fullpath.c_str())) {
		agent->sendMsg("That info file does not appear to exist.\n");
		return 0;
	}
	
	
	return 1;
}

