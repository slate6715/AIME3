#include <iostream>
#include <sstream>
#include <stdexcept>
#include "Social.h"
#include "MUD.h"
#include "actions.h"
#include "misc.h"
#include "global.h"
#include "Static.h"
#include "Getable.h"


const char *socflag_list[] = {"target1static", "target1getable", "adverb", NULL};

/*********************************************************************************************
 * Social (constructor)
 *
 *********************************************************************************************/
Social::Social(const char *id):
								Action(id)
{
	_typename = "Social";

}

// Copy constructor
Social::Social(const Social &copy_from):
								Action(copy_from),
								_messages(copy_from._messages),
								_amplifiers(copy_from._amplifiers),
								_socialflags(copy_from._socialflags)
{

}


Social::~Social() {

}

/*********************************************************************************************
 * saveData - Called by a child class to save Action-specific data into an XML document
 *
 *    Params:  entnode - This entity's node within the XML tree so attributes can be added to
 *                       it. This should be set up by a child class
 *             log - to log any errors
 *
 *********************************************************************************************/

void Social::saveData(pugi::xml_node &entnode) const {

	// First, call the parent version
	Social::saveData(entnode);

   // Add organism data (none yet)
   // pugi::xml_attribute idnode = entnode.append_attribute("id");
   // idnode.set_value(_id.c_str());

}

/*********************************************************************************************
 * loadData - Called by a child class to populate Action-specific data from an XML document
 *
 *    Params:  entnode - This entity's node within the XML tree so attributes can be drawn from
 *                       it
 *             log - to log any errors
 *
 *    Returns: 1 for success, 0 for failure
 *
 *********************************************************************************************/

int Social::loadData(pugi::xml_node &entnode) {

	std::stringstream errmsg;

	// First, call the parent function
	int results = 0;
	if ((results = Action::loadData(entnode)) != 1)
		return results;

   // Get the command aliases
   for (pugi::xml_node message = entnode.child("message"); message; message = message.next_sibling("message")) {

      // Get the messages that define this social
      pugi::xml_attribute attr = message.attribute("name");
      if (attr == nullptr) {
         errmsg << "Social '" << getID() << "' message node missing mandatory name field.";
         mudlog->writeLog(errmsg.str().c_str());
         return 0;
      }

		std::string msgname = attr.value();
		lower(msgname);

		auto m_ptr = _messages.insert(std::pair<std::string, std::string>(msgname, message.child_value()));

		if (!m_ptr.second) {
			errmsg << "Social '" << getID() << "' name '" << msgname << 
													"' unable to add message--possibly a duplicate name.";
			return 0;
		}
		
	}	

   for (pugi::xml_node amp = entnode.child("amplifier"); amp; amp = amp.next_sibling("amplifier")) {

      // Get the amplifiers, if any
      pugi::xml_attribute attr = amp.attribute("name");
      if (attr == nullptr) {
         errmsg << "Social '" << getID() << "' amplifier node missing mandatory name field.";
         mudlog->writeLog(errmsg.str().c_str());
         return 0;
      }

      std::string ampname = attr.value();
      lower(ampname);

      attr = amp.attribute("map");
      if (attr == nullptr) {
         errmsg << "Social '" << getID() << "' amplifier node missing mandatory map field.";
         mudlog->writeLog(errmsg.str().c_str());
         return 0;
      }

		std::string charstr = attr.value();
		if ((charstr.size() == 0) || (charstr.size() > 1) || (!isalnum(charstr[0]))) {
         errmsg << "Social '" << getID() << "' amplifier node map must be one alphanumeric character in size.";
         mudlog->writeLog(errmsg.str().c_str());
         return 0;
		}
	
      auto m_ptr = _amplifiers.insert(std::pair<std::string, amplifier>(ampname, amplifier(charstr[0], 
																							amp.child_value())));

      if (!m_ptr.second) {
         errmsg << "Social '" << getID() << "' name '" << ampname <<
                                       "' unable to add amplifier--possibly a duplicate name.";
         return 0;
      }

   }

	return 1;
}

/*********************************************************************************************
 * isSocialFlagSet - A faster version of isFlagSet which operates off the enum type for fast
 *						lookup, but only checks Action flags
 *
 *********************************************************************************************/

bool Social::isSocialFlagSet(social_flag stype) {
	return _socialflags[(unsigned int) stype];
}


/*********************************************************************************************
 * setFlagInternal - given the flag string, first checks the parent for the flag, then checks
 *                   this class' flags
 *
 *
 *********************************************************************************************/

bool Social::setFlagInternal(const char *flagname, bool newval) {
   if (Action::setFlagInternal(flagname, newval))
      return true;

	std::string flagstr = flagname;
   lower(flagstr);

   size_t i=0;
   while ((socflag_list[i] != NULL) && (flagstr.compare(socflag_list[i]) != 0))
      i++;

   if (socflag_list[i] == NULL)
      return false;

   _socialflags[i] = true;
	return true;
}

/*********************************************************************************************
 * isFlagSetInternal - given the flag string, first checks the parent for the flag, then checks
 *                   this class' flags
 *
 *    Params:  flagname - flag to set
 *             results - if found, what the flag is set to
 *
 *    Returns: true if the flag was found, false otherwise
 *
 *********************************************************************************************/

bool Social::isFlagSetInternal(const char *flagname, bool &results) {
   if (Action::isFlagSetInternal(flagname, results))
      return true;

   std::string flagstr = flagname;
   lower(flagstr);

   size_t i=0;
   while ((socflag_list[i] != NULL) && (flagstr.compare(socflag_list[i]) != 0))
      i++;

   if (socflag_list[i] == NULL)
      return false;

	results =_socialflags[i];
   return true;

}

/*********************************************************************************************
 * parseCommand - parses out the command based on this social's settings and preps to execute
 *
 *    Params:  cmd - the part of the command string after the action name itself
 *             errmsg - populated if an error is encountered
 *
 *    Returns: true if successful, false if an error was found
 *
 *********************************************************************************************/

bool Social::parseCommand(const char *cmd, std::string &errmsg) {
   size_t pos = 0;

	std::string buf = (cmd == NULL) ? "" : cmd;

   // Lone commands with these parse types are ok, exit early
   if ((getParseType() == Action::Single) ||
       ((getParseType() == Action::ActOptTarg) && (buf.size() == 0)))
      return true;

	// Check for invalid parse types for a social
	if ((getParseType() != Action::ActTarg) && (getParseType() != Action::ActOptTarg) &&
		 (getParseType() != Action::SocialStyle)) {
		std::string errmsg("ParseType not allowed with social '");
		errmsg += getID();
		errmsg += "'";
		mudlog->writeLog(errmsg.c_str());
		return false;
	}

	// Get our tokens
	while ((pos = parseToken(pos, buf)) < buf.size());

	if (numTokens() == 0)
		throw std::runtime_error("parseToken did not get a valid token for some reason.");

	// Look for an amplifier
	pos = buf.size() - 1;

	std::string poss_amp;
	if (numTokens() > 1)
		poss_amp = getToken(1);
	else
		poss_amp = getToken(0);

	lower(poss_amp);

	_selected_amp = _amplifiers.find(poss_amp);
	
	// If we found the amplifier match and this was the second token, no target so we're done
	if ((_selected_amp != _amplifiers.end()) && (numTokens() == 1)) {
		return true;
	}

   // We should have found an amplifier if they did act <targ> <amp>
   if ((_selected_amp == _amplifiers.end()) && (numTokens() == 2)) {
      errmsg = "I am not aware of that social adverb. Format: ";
      errmsg += getFormat();
      return false;
   }

	// Find the target
	setTarget1(findTarget(getToken(0), errmsg, 1));
	if (getTarget1() == nullptr) {
		return false;
	}

	// Now do a few more checks on Target1 constraints
	if (getTarget1() != nullptr) {
		if (isSocialFlagSet(Target1Static) && (std::dynamic_pointer_cast<Static>(getTarget1()) == nullptr)) {
			errmsg = "That only works with inanimate objects.\n";
			return false;
		}

      if (isSocialFlagSet(Target1Getable) && (std::dynamic_pointer_cast<Getable>(getTarget1()) == nullptr)) {
         errmsg = "That only works with items you can pick up.\n";
         return false;
      }

	}
	return true;
}

/*********************************************************************************************
 * execute - executes this action
 *
 *    Params:
 *
 *    Returns: 1 for success, 0 for failure
 *
 *********************************************************************************************/

int Social::execute() {

	StrFormatter sformat;
	std::string buf;

	// Load up our formatter with data
	getActor()->getGameName(buf);
	if (buf.size() == 0) {
		mudlog->writeLog("Actor name size of zero in Social::execute - should not have happened.");
		return 0;
	}
	buf[0] = toupper(buf[0]);
	
	sformat.addMap('n', buf.c_str());

	if (getTarget1() != nullptr) {
		// Target name
		sformat.addMap('t', getTarget1()->getGameName(buf));		
	}

	// Add the selected amp
	if (_selected_amp != _amplifiers.end()) {
		sformat.addMap(_selected_amp->second.charmap, _selected_amp->second.repl.c_str());
	}

	std::map<std::string, std::string>::iterator m_it;
	if (getTarget1() == nullptr) {

		// No target, send to the actor
		if ((m_it = _messages.find("actor")) != _messages.end()) {
			sformat.formatStr(m_it->second.c_str(), buf);
			getActor()->sendMsg(buf.c_str());
		}

      // No target, send to the room
      if ((m_it = _messages.find("room")) != _messages.end()) {
         sformat.formatStr(m_it->second.c_str(), buf);
         getActor()->getCurLoc()->sendMsg(buf.c_str(), getActor());
      }

	} else {
      if ((m_it = _messages.find("target_actor")) != _messages.end()) {
         sformat.formatStr(m_it->second.c_str(), buf);
         getActor()->sendMsg(buf.c_str());
      }
	
		if ((m_it = _messages.find("target")) != _messages.end()) {
         sformat.formatStr(m_it->second.c_str(), buf);
         getTarget1()->sendMsg(buf.c_str());
		}

      if ((m_it = _messages.find("target_room")) != _messages.end()) {
         sformat.formatStr(m_it->second.c_str(), buf);
         getActor()->getCurLoc()->sendMsg(buf.c_str(), getActor(), getTarget1());
      }

	}

   return 1;
}

