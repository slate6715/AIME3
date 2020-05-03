#ifndef PLAYER_H
#define PLAYER_H

#include <stack>
#include <queue>
#include <memory>
#include <mutex>
#include <libconfig.h++>
#include "Organism.h"
#include "Handler.h"
#include "TCPConn.h"

class MUD;

/***************************************************************************************
 * Player - a living entity that can usually be killed and sometimes moves around. Used
 *				  for players and NPCs. This is a generic class and should not be enstantiated
 *				  directly
 *
 ***************************************************************************************/
class Player : public Organism
{
public:
	
	Player(const char *id, std::unique_ptr<TCPConn> conn);
	Player(const Player &copy_from);

   virtual ~Player();

	virtual bool sendFile(const char *filename);
	virtual void sendMsg(const char *msg);
	virtual void sendMsg(std::string &msg);

   // Sends the prompt of the top message handler to the player
   virtual void sendPrompt();
	
	// Sends the player a blank the size of the prompt to clear it from a line
	virtual void clearPrompt();

	// Displays the current location to the user
	virtual void sendCurLocation();

	// Displays the exits of the current location to the user
	virtual void sendExits();

	// configures the connecting user for entering the MUD
	void welcomeUser(libconfig::Config &mud_cfg, std::shared_ptr<Player> thisplr);

	// Loops through the player's connection, handling data
	void handleConnection();

	bool popCommand(std::string &cmd);

	// Sends the command through the current message handler on top of the stack
	int handleCommand(std::string &cmd);

	// Removes the top handler, calling a postPop function beforehand and returning
   // data from the handler as appropriate
	void popHandler(std::vector<std::string> &results);

   // Functions for loading and saving user info to disk
   int loadUser(const char *userdir, const char *username);
   bool saveUser(const char *userdir) const;
   virtual void saveData(pugi::xml_node &entnode) const;
   virtual int loadData(pugi::xml_node &entnode);


	// Create a password for a new user or to change an existing user's password
	void createPassword(const char *cleartext);

	bool checkPassword(const char *cleartext);

protected:

	// For any future player flags
   virtual bool setFlagInternal(const char *flagname, bool newval);
   virtual bool isFlagSetInternal(const char *flagname, bool &results);
	


private:

	void formatForTelnet(const std::string &unformatted, std::string &formatted);
	void generatePasswdHash(const char *cleartext, std::vector<unsigned char> &buf,
                                                                           std::vector<unsigned char> &salt);

	std::unique_ptr<TCPConn> _conn;

	std::stack<std::unique_ptr<Handler>> _handler_stack;

	// A queue of commands read in from the connection. Mutex controls access as the socket
   // handling thread could create race conditions and queue is not thread-safe
	std::mutex _cmd_mutex;
	std::queue<std::string> _commands;

	// User-specific formatting variables
	bool _use_color = true;
	unsigned int _wrap_width = 80;
	unsigned int _last_wrap = 0;

	// **************** Player-specific variables ************************

	// The salt/hash generated from the password
	std::vector<unsigned char> _passwd_hash;

	// **** Game-specific attributes ****

};


#endif
