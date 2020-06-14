#include <fstream>
#include <sstream>
#include <iostream>
#include <bitset>
#include <argon2.h>
#include <string.h>
#include <boost/algorithm/hex.hpp>
#include <memory>
#include "Player.h"
#include "LoginHandler.h"
#include "GameHandler.h"
#include "PageHandler.h"
#include "ActionMgr.h"
#include "Location.h"
#include "misc.h"
#include "../external/pugixml.hpp"
#include "global.h"
#include "Getable.h"
#include "Equipment.h"

// Defines the password hash/salt bytelength
const unsigned int hashlen = 16;
const unsigned int saltlen = 8;

const char *pflag_list[] = {"nochat", NULL};

/*********************************************************************************************
 * Player (constructor)
 *	
 *		Params:	id - unique id of this player (player:<username>)
 *					conn - A constructed pointer (sink) to a TCPConn object that will be destroyed
 *						    when this player object is destroyed
 *
 *********************************************************************************************/
Player::Player(const char *id, std::unique_ptr<TCPConn> conn):
																Organism(id),
																_conn(std::move(conn)),
																_handler_stack(),
																_cmd_mutex(),
																_commands(),
																_use_color(true),
																_passwd_hash()
{
	_typename = "Player";
}

// Called by child class
Player::Player(const Player &copy_from):
								Organism(copy_from),
								_conn(new TCPConn(*(copy_from._conn))),
								_handler_stack(),
								_cmd_mutex(),
								_commands(copy_from._commands),
								_use_color(copy_from._use_color),
								_passwd_hash(copy_from._passwd_hash)
{
}


Player::~Player() {

}

/*********************************************************************************************
 * sendFile - takes a filename and opens/sends the file to the user. Useful for things like 
 *			     the welcome message or motd, help files, info files, etc
 *
 *		Returns: true if successfully read, false otherwise
 *
 *********************************************************************************************/

bool Player::sendFile(const char *filename) {
	
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
		sendMsg("\n");
		sendMsg(buf);

		readfile.close();
	}
	catch (std::ifstream::failure &e) {
		return false;
	}
	return true;
}

/*********************************************************************************************
 * sendMsg - mainly just adds this string to the send queue in TCPConn
 *
 *
 *********************************************************************************************/

void Player::sendMsg(const char *msg, std::shared_ptr<Physical> exclude) {
	std::string unformatted = msg;
	sendMsg(unformatted, exclude);
}

void Player::sendMsg(std::string &msg, std::shared_ptr<Physical> exclude) {
	(void) exclude;

	std::string formatted;
	formatForTelnet(msg, formatted);
	_conn->addOutput(formatted.c_str());	
}

/*std::ostream &Player::operator << (std::ostream &out, const Player &p) {

}*/

/*********************************************************************************************
 * welcomeUser - initializes the player object and starts the sequence of logging in the user
 *
 *
 *********************************************************************************************/

void Player::welcomeUser(libconfig::Config &mud_cfg, std::shared_ptr<Player> thisplr) {

	// First, place a GameHandler on the stack that should not be popped
	_handler_stack.push(std::unique_ptr<Handler>(new GameHandler(thisplr)));

	// Place a pagehandler to display the MOTD after they login
	PageHandler *phandler = new PageHandler(thisplr, 60);
	std::string loginfile, infodir;
	mud_cfg.lookupValue("datadir.infodir", infodir);
	infodir += "/";
	mud_cfg.lookupValue("infofiles.logged_in", loginfile);
	infodir += loginfile;
	phandler->addFileContent(infodir.c_str());

	_handler_stack.push(std::unique_ptr<Handler>(phandler));

	// Now place a login handler on the stack
	_handler_stack.push(std::unique_ptr<Handler>(new LoginHandler(thisplr, mud_cfg)));
	_handler_stack.top()->postPush();
}

/*********************************************************************************************
 * handleConnection - checks the connection for input and sends any output
 *
 *
 *********************************************************************************************/

void Player::handleConnection(time_t timeout) {
	// Send all data and receive data
	_conn->handleConnection(timeout);

	// Extract received data into a command queue
	std::string buf, left, right;
	if (_conn->getUserInput(buf) > 0) {
	
		// Lock down the commands queue so we can add to it
		std::lock_guard<std::mutex> guard(_cmd_mutex);
	
		// Get commands while there are still newlines
		while (split(buf, left, right, '\n')) {
			clrNewlines(left);

			// Push the commands onto the player's command queue
			_commands.push(left);
			buf = right;
		}
	}
}


/*********************************************************************************************
 * popCommand - Removes the next command from the queue and populates the parameter
 *
 *		Params:	buf - command overwrites the content of this string
 *
 *		Returns: true if a command was found, false otherwise
 *
 *********************************************************************************************/

bool Player::popCommand(std::string &buf) {
	
	// If no commands found
	if (_commands.size() == 0)
		return false;

	// Make sure we can access the commands queue in a thread-safe way
	std::lock_guard<std::mutex> guard(_cmd_mutex);

	// Get the next command
	buf = _commands.front();
	_commands.pop();

	return true;
}

// this define and table was taken from abermud code written by Eric
// from northern lights. It was just such a cool way to do it that I
// lifted it

const char color_table[] =
{        
     '\0',   '\0',   '4',    '6',
     '\0',   '\0',   '\0',   '2',
     '\0',   '\0',   '\0',   '\0',
     '0',    '5',    '\0',   '\0',
     '\0',   '\0',   '1',    '\0',
     '\0',   '\0',   '\0',   '7',
     '\0',   '3',    '\0',   '\0',
     '\0',   '\0',   '\0',   '\0',
     '\0',   '\0',   '4',    '6',
     '\0',   '\0',   '\0',   '2',
     '\0',   '\0',   '\0',   '\0',
     '0',    '5',    '\0',   '\0',
     '\0',   '\0',   '1',    '\0',
     '\0',   '\0',   '\0',   '7',
     '\0',   '3',    '\0',   '\0',
     '\0',   '\0',   '\0',   '\0',
};

#define colorcode(x) ( (x>=64) ? color_table[x-64] : 0 )

/*********************************************************************************************
 * formatForTelnet - Converts the outgoing text from MUD format to a format meeting RFC5198
 *						   Telnet protocol specs w/ ANSI colorcodes, which should work for mud clients too
 *
 *    Params:  unformatted - string buffer with the unformatted text
 *					formatted - string buffer to contain the new formatted text (should not be the
 *                         same buffer as unformatted)
 *
 *********************************************************************************************/

void Player::formatForTelnet(const std::string &unformatted, std::string &formatted) {
	// Set up a bitset class to contain the characters we're looking for
	std::bitset<256> keychars;
	keychars['&'] = true;
	keychars['\n'] = true;

	// Keep track of the last colorcode so we can turn it off
	
	// Reserve some space in the formatted string, assuming 20% greater than unformatted
	formatted.clear();
	formatted.reserve(unformatted.size() * 1.2);

	std::string colorstr;

	unsigned int lastpos = 0;
	for (unsigned int i=0; i<unformatted.size(); i++) {

      // if we find a '\r', then restart our wrapping and check the next for a \n 
      if ((unformatted[i] == '\r') || (unformatted[i] == '\n'))
		{
			_last_wrap = 0;
			continue;
		}

		// If we're at our word-wrap location, wrap it
		if ((_wrap_width != 0) && (_last_wrap >= _wrap_width)) {

			// Step backwards to find a space
			unsigned int j=i;
			while ((j > 0) && (j > lastpos) && (j--)) {
				if (unformatted[j] == ' ') {
					formatted.append(unformatted, lastpos, j-lastpos);
					formatted.append("\r\n");
					lastpos = j+1;
					_last_wrap = 0;
					i = lastpos;
					continue;
				}
			}

			// We may have a situation where the colorcode was placed right before wrap		
			if ((j == lastpos) && ((i - lastpos) < _wrap_width)) {
				formatted.append("\r\n");
				lastpos = j;
				_last_wrap = 0;
				i = j;
				continue;
			}
	
			// We did not find a space to wrap. Just chop it at wraplength
			else if (j == lastpos) {
				formatted.append(unformatted, lastpos, i-lastpos+1);
				formatted.append("\r\n");
				lastpos = i+1;
				_last_wrap = 0;
				i = lastpos;
				continue;
			}	
		}

		// Keep going while it's just a regular character
		if (!keychars[(std::size_t) unformatted[i]]) {
			_last_wrap++;
			continue;
		}

		// /n should be preceeded by a \r
		if (unformatted[i] == '\n') {
			_last_wrap = 0;
			if ((i == 0) || (unformatted[i-1] != '\r')) {
				if ((i - lastpos) > 1)
					formatted.append(unformatted, lastpos, i-lastpos);
				formatted.append("\r\n");
				lastpos = i + 1;
			} 
		}
		// Ampersands could be a colorcode or an && 
		else if (unformatted[i] == '&') {
			if (i+1 == unformatted.size())
				continue;

			// Change double && to single &
			if (unformatted[i+1] == '&') {
				formatted.append(unformatted, lastpos, i-lastpos);
				lastpos = i + 2;
				_last_wrap++;
			}
			// Turn off colorcodes
			else if (unformatted[i+1] == '*') {
				formatted.append(unformatted, lastpos, i-lastpos);
				if (_use_color)
					formatted.append("\033[1;0m");
				lastpos = i+2;
			}
         // Turn on bold
         else if (unformatted[i+1] == '^') {
            formatted.append(unformatted, lastpos, i-lastpos);
            if (_use_color)
               formatted.append("\033[1;1m");
            lastpos = i+2;
         }
         // Turn on italics
         else if (unformatted[i+1] == '~') {
            formatted.append(unformatted, lastpos, i-lastpos);
            if (_use_color)
               formatted.append("\033[1;3m");
            lastpos = i+2;
         }
         // Turn on underline
         else if (unformatted[i+1] == '_') {
            formatted.append(unformatted, lastpos, i-lastpos);
            if (_use_color)
               formatted.append("\033[1;4m");
            lastpos = i+2;
         }
			// Blinking
         else if (unformatted[i+1] == '@') {
            formatted.append(unformatted, lastpos, i-lastpos);
            if (_use_color)
               formatted.append("\033[1;5m");
            lastpos = i+2;
         }

			// A plus means set text color, negative background color
			else if ((unformatted[i+1] == '+') || (unformatted[i+1] == '-')) {
				if (i+2 == unformatted.size())
					continue;

				if (lastpos != i)
					formatted.append(unformatted, lastpos, i-lastpos);

				if ((_use_color) && (colorcode((int) unformatted[i+2]) != '\0')) {
					// Foreground color?
					colorstr = "\033[1;30m";

					// No, background, change it!
					if (unformatted[i+1] == '-')
						colorstr[4] = '4';

					colorstr[5] = colorcode((int) unformatted[i+2]);

					formatted.append(colorstr);
				}
				lastpos = i+3;
			} 
			// '=' means both foreground and background colors
			else if (unformatted[i+1] == '=') {
				if (i+3 >= unformatted.size())
					continue;

				if (lastpos != i)
					formatted.append(unformatted, lastpos, i-lastpos);
				
				if ((_use_color) && ((colorcode((int) unformatted[i+2]) != '\0') && 
											(colorcode((int) unformatted[i+3]) != '\0'))) {
					colorstr = "\033[1;40;30m";
					colorstr[5] = colorcode((int) unformatted[i+2]);
					colorstr[8] = colorcode((int) unformatted[i+3]);
					formatted.append(colorstr);
				}
				lastpos = i+4;
			}
			i = lastpos-1;

		}
	}
	if (lastpos < unformatted.size()) {
		formatted.append(unformatted, lastpos, unformatted.size()-lastpos);
	}
}

/*********************************************************************************************
 * sendPrompt - Sends the prompt of the message handler on top of the stack to the user
 *
 *
 *********************************************************************************************/

void Player::sendPrompt() {
	std::string prompt;

	Handler &cur_handler = *(_handler_stack.top());
	cur_handler.getPrompt(prompt);

	sendMsg(prompt);
}

/*********************************************************************************************
 * clearPrompt - Returns the cursor to the beginning of the line and wipes out the prompt, again
 *				     returning it to the start for new text.
 *
 *
 *********************************************************************************************/

void Player::clearPrompt() {
   std::string prompt;

   Handler &cur_handler = *(_handler_stack.top());
   cur_handler.getPrompt(prompt);

	std::string clrprompt;
	clrprompt.assign(prompt.size(), ' ');
	clrprompt.insert(0, "\r");
	clrprompt += "\r";
   sendMsg(clrprompt); 
}


/*********************************************************************************************
 * sendCurLoc - displays all the pertinant information about the current location to the player
 *
 *
 *********************************************************************************************/

void Player::sendCurLocation() {
	std::shared_ptr<Location> locptr;

	if ((locptr = std::dynamic_pointer_cast<Location>(getCurLoc())) == nullptr) {
		sendMsg("You appear to be trapped inside a non-location entity. Speak to an Admin!\n");
		return;
	}

	sendMsg("\n");
	sendMsg(locptr->getTitle());
	sendMsg("\n\n");
	sendMsg(locptr->getDesc());

	sendLocContents();
	sendMsg("\n");

	sendExits();
	sendMsg("\n");
}

/*********************************************************************************************
 * sendExits - displays all the visible exits to the user for their location
 *
 *
 *********************************************************************************************/

void Player::sendExits() {
   std::shared_ptr<Location> locptr;

   if ((locptr = std::dynamic_pointer_cast<Location>(getCurLoc())) == nullptr) {
      sendMsg("You appear to be trapped inside a non-location entity. Speak to an Admin!\n");
      return;
   }

	std::string buf;
	sendMsg(locptr->getExitsStr(buf));
}


/*********************************************************************************************
 * sendLocContents - displays the getable and organism contents in the room
 *
 *
 *********************************************************************************************/

void Player::sendLocContents() {
   std::shared_ptr<Location> locptr;

   if ((locptr = std::dynamic_pointer_cast<Location>(getCurLoc())) == nullptr) {
      sendMsg("You appear to be trapped inside a non-location entity. Speak to an Admin!\n");
      return;
   }

	std::string buf;
	sendMsg(locptr->listContents(buf, this));
}

/*********************************************************************************************
 * handleCommand - sends the command to the top message handler for it to execute for this
 *                 player
 *
 *********************************************************************************************/

int Player::handleCommand(std::string &cmd) {


	// Execute the command
	_handler_stack.top()->handleCommand(cmd);

	if (_handler_stack.top()->handler_state != Handler::Active)
		return 1;

	return 0;
}

/*********************************************************************************************
 * popHandler - Calls the prePop function to clean up and get results from the handler, then
 *					 removes it from the stack
 *
 *********************************************************************************************/

void Player::popHandler(std::vector<std::string> &results) {
	if (_handler_stack.size() <= 1) {
		throw std::runtime_error("Attempted to pop last handler or no handlers in the stack, "
										 "which should not have happened");
	}

	_handler_stack.top()->prePop(results);

	_handler_stack.pop();

	// Execute any code when this handler activates (like with the PageHandler)
	if (_handler_stack.top()->activate())
		popHandler(results);

}


/*********************************************************************************************
 * loadUser - attempts to load the user into the given Player object
 *
 *    Params:  username - self-explanatory
 *             plr - an existing Player class that will be populated (note: id will not be
 *                   changed and should not be until authentication happens)
 *
 *    Returns: 1 if loaded, 0 if not found
 *
 *********************************************************************************************/

int Player::loadUser(const char *userdir, const char *username) {
   pugi::xml_document userfile;
	std::stringstream errmsg;

   std::string filename = userdir;
   std::string user = username;
   lower(user);
	filename += "/";
   filename += user;
   filename += ".xml";

   pugi::xml_parse_result result = userfile.load_file(filename.c_str());

   if (!result) {
      return 0;
   }

	pugi::xml_node pnode = userfile.child("player");
	if (pnode == nullptr) {
		errmsg << "Corrupted player file for player " << user;
		mudlog->writeLog(errmsg.str().c_str());
		return -1;
	}
	if (!loadData(pnode)) {
		std::stringstream errmsg;
		errmsg << "Player '" << user << "' save file not in the proper format.";
		mudlog->writeLog(errmsg.str().c_str());

		sendMsg("Your save file has been corrupted and cannot be loaded. Contact an admin.\n");
		return -1; 
	}
	
   return 1;
}

/*********************************************************************************************
 * saveUser - saves the user data to a file
 *
 *    Params:  username - self-explanatory
 *
 *    Returns: 1 if loaded, 0 if not found
 *
 *********************************************************************************************/

bool Player::saveUser(const char *userdir) const {

	// Populate an XML document with player data, then save it to the file

   pugi::xml_document userfile;
	std::string buf;
	
	// Username needs to be accurately stored in the id field
   std::string filename;
	filename += userdir;
	filename += "/";
	filename += getNameID(buf);
	filename += ".xml";

	pugi::xml_node node = userfile.append_child("player");

	saveData(node);	

	if (!userfile.save_file(filename.c_str())) {
		return false;
	}
   return true;
}

/*********************************************************************************************
 * generatePasswdHash - takes a plaintext password and generate a password hash based on that
 *          passwd. If the salt parameter is empty, this function will generate a random salt. 
 *          If it's not empty, that salt will be used instead. The salt must be stored as-is
 *          in the user account in order to be able to replicate the hash.
 *
 *    Params:  cleartext - the cleartext password to hash
 *					buf - vector buffer that stores the password hash
 *					salt - vector salt - empty = generate random salt, otherwise should be the
 *                    appropriate salt length
 *
 *
 *********************************************************************************************/

void Player::generatePasswdHash(const char *cleartext, std::vector<unsigned char> &buf, 
																									std::vector<unsigned char> &salt)
{
   uint8_t hashbuf[hashlen];

   srand(time(0));

   // Generate a random salt string
   uint8_t saltbuf[saltlen];

   // If the user provided a salt, use that...otherwise, randomly generate one
   if (salt.size() != saltlen) {
		salt.clear();
		salt.reserve(saltlen);
      for (unsigned int i=0; i<saltlen; i++) {
         saltbuf[i] = ((rand() % 93)+33);  // ascii characters ! through ~
			salt.push_back(saltbuf[i]);
		}
   }
	else
		std::copy(salt.begin(), salt.end(), saltbuf);

   const uint32_t t_cost = 2;          // 1 pass computation
   const uint32_t m_cost = (1<<16);    // 64 MB memory usage
   const uint32_t parallelism = 1;     // number of threads and lanes

   // hash and place into the hashstr array
   argon2i_hash_raw(t_cost, m_cost, parallelism, cleartext, strlen(cleartext), saltbuf, saltlen, hashbuf, hashlen);

   buf.clear();
   buf.reserve(hashlen);
   for (unsigned int i=0; i<hashlen; i++)
      buf.push_back(hashbuf[i]);

}

/*********************************************************************************************
 * createPassword - Creates a new password hash and saves it into this user's password field. 
 *
 *    Params:  cleartext - the cleartext password to hash
 *
 *
 *********************************************************************************************/

void Player::createPassword(const char *cleartext)
{
	std::vector<unsigned char> hash, salt;

	generatePasswdHash(cleartext, hash, salt);
	_passwd_hash.clear();
	_passwd_hash.assign(salt.begin(), salt.end());
	_passwd_hash.insert(std::end(_passwd_hash), std::begin(hash), std::end(hash));	
}

/*********************************************************************************************
 * checkPassword - Hashes a plaintext password and compares against the stored password hash
 *
 *    Params:  cleartext - the cleartext password to compare 
 *
 *		Returns: true for passwords match, false otherwise
 *
 *********************************************************************************************/

bool Player::checkPassword(const char *cleartext)
{
   std::vector<unsigned char> salt, hash, comparehash;

   salt.assign(_passwd_hash.begin(), _passwd_hash.begin() + saltlen);

	generatePasswdHash(cleartext, hash, salt);
	comparehash.assign(_passwd_hash.begin() + saltlen, _passwd_hash.end());

	return (hash == comparehash);
}

/*********************************************************************************************
 * saveData - Called by a child class to save Player-specific data into the XML tree
 *
 *    Params:  entnode - This entity's node within the XML tree so attributes can be added to
 *                       it. This should be set up by a child class
 *
 *********************************************************************************************/

void Player::saveData(pugi::xml_node &entnode) const {
   // First, call the parent version
   Organism::saveData(entnode);

   // Saving the password, we need to convert it to hex
	std::string hexstr;
	hexstr.assign(_passwd_hash.size()*2, '0');
	boost::algorithm::hex(_passwd_hash.begin(), _passwd_hash.end(), hexstr.begin());

   pugi::xml_attribute idnode = entnode.append_attribute("passwd");
   idnode.set_value(hexstr.c_str());

	idnode = entnode.append_attribute("wrap_width");
	idnode.set_value(_wrap_width);
}

/*********************************************************************************************
 * loadData - Called by a child class to populate Player-specific data from an XML document
 *
 *    Params:  entnode - This entity's node within the XML tree so attributes can be drawn from
 *                       it
 *
 *    Returns: 1 for success, 0 for failure
 *
 *********************************************************************************************/

int Player::loadData(pugi::xml_node &entnode) {

	std::stringstream errmsg;

   // First, call the parent function
   int results = 0;
   if ((results = Organism::loadData(entnode)) != 1)
      return results;
	

	// Read in the password hash		
   pugi::xml_attribute attr = entnode.attribute("passwd");
   if (attr == nullptr) {
      mudlog->writeLog("Player save file missing mandatory 'passwd' field.", 2);
      return 0;
   }
	std::string pwdhash = attr.value();
	_passwd_hash.assign(hashlen+saltlen, 0);
	boost::algorithm::unhex(pwdhash.begin(), pwdhash.end(), _passwd_hash.begin());

	// Read in the wrap_width for their terminal (optional)
	attr = entnode.attribute("wrap_width");
	if (attr != nullptr) {
		try {
			setWrapWidth((unsigned int) std::atoi(attr.value()));
		} catch (std::exception &e) {
			errmsg << "Unexpected error reading in wrap_width for player '" << getID() << "': " << e.what();
			mudlog->writeLog(errmsg.str().c_str());
		}
	}

	
	return 1;
}

/*********************************************************************************************
 * setFlagInternal - given the flag string, first checks the parent for the flag, then checks
 *							this class' flags
 *
 *
 *********************************************************************************************/

bool Player::setFlagInternal(const char *flagname, bool newval) {
	if (Organism::setFlagInternal(flagname, newval))
		return true;

	// Here we would look for player flags
   std::string flagstr = flagname;
   lower(flagstr);

   size_t i=0;
   while ((pflag_list[i] != NULL) && (flagstr.compare(pflag_list[i]) != 0))
      i++;

   if (pflag_list[i] == NULL)
      return false;

   _pflags[i] = true;
   return true;
}

/*********************************************************************************************
 * isFlagSetInternal - given the flag string, first checks the parent for the flag, then checks
 *                   this class' flags 
 *
 *		Params:	flagname - flag to set
 *					results - if found, what the flag is set to
 *	
 *    Returns: true if the flag was found, false otherwise
 *
 *********************************************************************************************/

bool Player::isFlagSetInternal(const char *flagname, bool &results) {
	if (Organism::isFlagSetInternal(flagname, results))
		return true;

   std::string flagstr = flagname;
   lower(flagstr);

   size_t i=0;
   while ((pflag_list[i] != NULL) && (flagstr.compare(pflag_list[i]) != 0))
      i++;

   if (pflag_list[i] == NULL)
      return false;

   results =_pflags[i];
   return true;
	
}

/*********************************************************************************************
 * listContents - preps a string with a list of visible items in this player's container (inventory)
 *
 *
 *********************************************************************************************/

const char *Player::listContents(std::string &buf, const Physical *exclude) const {
	(void) exclude;
   auto cit = _contained.begin();

	if (_contained.size() == 0) {
		buf = "Nothing.\n";
		return buf.c_str();
	}

   // Show getables first
   for (cit = _contained.begin(); cit != _contained.end(); cit++) {
      std::shared_ptr<Getable> gptr = std::dynamic_pointer_cast<Getable>(*cit);

      if (gptr == nullptr)
         continue;

      buf += gptr->getTitle();

		// If it is a worn item, list where it is worn. This gets a bit complicated though so we look
		// for patterns
		std::shared_ptr<Equipment> eptr = std::dynamic_pointer_cast<Equipment>(gptr);
		if (eptr != nullptr) {
				
		}
	
      buf += "\n";
   }

   return buf.c_str();
}

/*********************************************************************************************
 * clearNonSaved - removes items from the player's inventory that are not coded to be saved
 *						 upon quit
 *
 *		Params: death - is this due to a death, which may cause more items to be lost
 *
 *********************************************************************************************/

void Player::clearNonSaved(bool death) {
	(void) death;

	auto c_it = _contained.begin();
	while (c_it != _contained.end()) {
		// Remove items that are not saved and not to be dropped
	

		// Drop items that are not to be saved
		//if ((*c_it)->isFlagSet("NoSave")) {
		//	(*c_it)->movePhysical(getCurLoc(), *c_it);
		//}

		// trigger a special for any unique requirements to save or not save (TODO)

	}
	
}

/*********************************************************************************************
 * quit - disconnects the player and prepares the object to be removed
 *
 *
 *********************************************************************************************/

void Player::quit() {
	_conn->startDisconnect();

	_quitting = true;
}

/*********************************************************************************************
 * purgePhysical - Removes all references to the parameter from the Entities in the database so
 *               it can be safely removed
 *
 *    Returns: number of references to this object cleared
 *
 *********************************************************************************************/

size_t UserMgr::purgePhysical(std::shared_ptr<Physical> item) {
   size_t count = 0;

   // Loop through all items, purging the entity
   auto plr_it = _db.begin();
   for ( ; plr_it != _db.end(); plr_it++) {
      count += plr_it->second->purgePhysical(item);
   }
   return count;
}

