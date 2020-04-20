#include "Player.h"
#include "Handler.h"

/*********************************************************************************************
 * Player (constructor)
 *	
 *		Params:	id - unique id of this player (player@<username>)
 *					conn - A constructed pointer (sink) to a TCPConn object that will be destroyed
 *						    when this player object is destroyed
 *
 *********************************************************************************************/
Player::Player(const char *id, std::unique_ptr<TCPConn> conn):
								Organism(id) 
{


}

// Called by child class
Player::Player(const Player &copy_from):
								Organism(copy_from),
								unique_ptr<TCPConn>(new TCPConn(_conn))
{

}


Player::~Player() {

}

void Player::sendFile(const char *filename) {
	
}

/*********************************************************************************************
 * welcomeUser - initializes the player object and starts the sequence of logging in the user
 *
 *
 *********************************************************************************************/

void Player::welcomeUser() {

	// First, place a GameHandler on the stack that should not be popped
	_handler_stack.push(std::unique_ptr<Handler>(new GameHandler()));

	
}
