#include <exceptions.h>
#include <cstring>
#include <iostream>
#include "Handler.h"
#include "strfuncts.h"
#include "Player.h"


/*********************************************************************************************
 * Handler (constructor)
 *	
 *
 *********************************************************************************************/
Handler::Handler(Player &plr):
							handler_state(Active),
							return_val(""),
							_plr(plr)
{
}

// Called by child class
Handler::Handler(const Handler &copy_from):
							handler_state(copy_from.handler_state),
							return_val(copy_from.return_val),
							_plr(copy_from._plr)
{

}


Handler::~Handler() {

}


