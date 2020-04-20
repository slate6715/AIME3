#ifndef PLAYER_H
#define PLAYER_H

#include "Organism.h"
#include "Handler.h"

/***************************************************************************************
 * Player - a living entity that can usually be killed and sometimes moves around. Used
 *				  for players and NPCs. This is a generic class and should not be enstantiated
 *				  directly
 *
 ***************************************************************************************/
class Player : public Organism
{
public:
	
	Player(const char *id, std::unique_ptr<Player> conn);
	Player(const Player &copy_from);

   virtual ~Player();

	void welcomeUser();

protected:

private:

	std::unique_ptr<Player> _conn;

	std::stack<std::unique_ptr<Handler>> _handler_stack;
};


#endif
