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

	void welcomeUser(const char *welcome_file);

	bool popCommand(std::string &cmd);

protected:

private:

	void formatForTelnet(const std::string &unformatted, std::string &formatted);

	std::unique_ptr<TCPConn> _conn;

	std::stack<std::unique_ptr<Handler>> _handler_stack;

	// A queue of commands read in from the connection. Mutex controls access as the socket
   // handling thread could create race conditions and queue is not thread-safe
	std::mutex _cmd_mutex;
	std::queue<std::string> _commands;

	// User-specific formatting variables
	bool _use_color = true;

	LogMgr &_log;
};


#endif
