#ifndef PLAYER_H
#define PLAYER_H

#include <stack>
#include <queue>
#include <memory>
#include <mutex>
#include "Organism.h"
#include "Handler.h"
#include "LogMgr.h"
#include "TCPConn.h"

/***************************************************************************************
 * Player - a living entity that can usually be killed and sometimes moves around. Used
 *				  for players and NPCs. This is a generic class and should not be enstantiated
 *				  directly
 *
 ***************************************************************************************/
class Player : public Organism
{
public:
	
	Player(const char *id, std::unique_ptr<TCPConn> conn, LogMgr &log);
	Player(const Player &copy_from);

   virtual ~Player();

	void sendFile(const char *filename);
	void sendMsg(const char *msg);
	void sendMsg(std::string &msg);
// 	friend std::ostream & operator<<(std::ostream &out, const Player &p);

	void handleConnection();

	void welcomeUser(const char *welcome_file, const char *userdir);

	// Sends the prompt of the top message handler to the player
	void sendPrompt();

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
   virtual int loadData(LogMgr &log, pugi::xml_node &entnode);


	// Create a password for a new user or to change an existing user's password
	void createPassword(const char *cleartext);

	bool checkPassword(const char *cleartext);
	

protected:

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

	LogMgr &_log;

	// **************** Player-specific variables ************************

	// The salt/hash generated from the password
	std::vector<unsigned char> _passwd_hash;
};


#endif
