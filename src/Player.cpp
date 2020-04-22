#include <fstream>
#include <sstream>
#include <bitset>
#include <argon2.h>
#include <string.h>
#include "Player.h"
#include "Handler.h"
#include "strfuncts.h"
#include "../external/pugixml.hpp"

// Defines the password hash/salt bytelength
const unsigned int hashlen = 16;
const unsigned int saltlen = 8;

/*********************************************************************************************
 * Player (constructor)
 *	
 *		Params:	id - unique id of this player (player@<username>)
 *					conn - A constructed pointer (sink) to a TCPConn object that will be destroyed
 *						    when this player object is destroyed
 *
 *********************************************************************************************/
Player::Player(const char *id, std::unique_ptr<TCPConn> conn, LogMgr &log):
																Organism(id),
																_conn(std::move(conn)),
																_handler_stack(),
																_cmd_mutex(),
																_commands(),
																_use_color(true),
																_log(log),
																_passwd_hash()
{

}

// Called by child class
Player::Player(const Player &copy_from):
								Organism(copy_from),
								_conn(new TCPConn(*(copy_from._conn))),
								_handler_stack(),
								_cmd_mutex(),
								_commands(copy_from._commands),
								_use_color(copy_from._use_color),
								_log(copy_from._log),
								_passwd_hash(copy_from._passwd_hash)
{
}


Player::~Player() {

}

/*********************************************************************************************
 * sendFile - takes a filename and opens/sends the file to the user. Useful for things like 
 *			     the welcome message or motd, help files, info files, etc
 *
 *
 *********************************************************************************************/

void Player::sendFile(const char *filename) {
	
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
		sendMsg(buf);

		readfile.close();
	}
	catch (std::ifstream::failure &e) {
		std::stringstream msg;
		
		msg << "Attempted to open/send file '" << filename << "' to player '" << getID() << "' failed. Error: " << e.what();
		_log.writeLog(msg.str().c_str());	
	}
	
}

/*********************************************************************************************
 * sendMsg - mainly just adds this string to the send queue in TCPConn
 *
 *
 *********************************************************************************************/

void Player::sendMsg(const char *msg) {
	std::string unformatted = msg;
	sendMsg(unformatted);
}

void Player::sendMsg(std::string &msg) {

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

void Player::welcomeUser(const char *welcome_file, const char *userdir) {

	// First, place a GameHandler on the stack that should not be popped
	_handler_stack.push(std::unique_ptr<Handler>(new GameHandler(*this)));

	// Now place a login handler on the stack
	_handler_stack.push(std::unique_ptr<Handler>(new LoginHandler(*this, userdir)));

	// Send him the welcome message in the welcome file (location per the config file)
	sendFile(welcome_file);	
	
	sendPrompt();
}

/*********************************************************************************************
 * handleConnection - checks the connection for input and sends any output
 *
 *
 *********************************************************************************************/

void Player::handleConnection() {
	// Send all data and receive data
	_conn->handleConnection();

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

		// Keep going while it's just a regular character
		if (!keychars[(std::size_t) unformatted[i]])
			continue;

		// /n should be preceeded by a \r
		if (unformatted[i] == '\n') {
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
 * handleCommand - sends the command to the top message handler for it to execute for this
 *                 player
 *
 *********************************************************************************************/

void Player::handleCommand(std::string &cmd) {

	// Execute the command
	_handler_stack.top()->handleCommand(cmd);


	// Based on the result of the handler, we may need to do a few things here	
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

   std::string filename = userdir;
   std::string user = username;
   lower(user);
   filename += user;
   filename += ".xml";

   pugi::xml_parse_result result = userfile.load_file(filename.c_str());

   if (!result) {
      return 0;
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

bool Player::saveUser() const {

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
