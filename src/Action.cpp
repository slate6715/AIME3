#include <iostream>
#include "Action.h"
#include "MUD.h"


/*********************************************************************************************
 * Action (constructor) - Called by a child class to initialize any Action elements. Used to
 *								  create a scripted version of an Action
 *
 *********************************************************************************************/
Action::Action(const char *id):
								Entity(id) 
{


}

/*********************************************************************************************
 * Action (constructor) - Called by a child class to initialize any Action elements. Used to
 *                        create a hard-coded version of a class
 *
 *		Params:	id - Entity-stored id of this action--also the name that it's referred to
 *					act_ptr - a hard-coded function pointer that is called by this Action
 *
 *********************************************************************************************/
Action::Action(const char *id, int (*act_ptr)(MUD &, Action &), parse_type ptype):
											Entity(id),	
											_atype(Hardcoded),
											_ptype(ptype),
											_act_ptr(act_ptr),
											_agent(nullptr)
				
{

}

// Copy constructor
Action::Action(const Action &copy_from):
								Entity(copy_from),
								_atype(copy_from._atype),
								_ptype(copy_from._ptype),
								_act_ptr(copy_from._act_ptr),
								_agent(copy_from._agent)
{

}


Action::~Action() {

}

/*********************************************************************************************
 * saveData - Called by a child class to save Action-specific data into an XML document
 *
 *    Params:  entnode - This entity's node within the XML tree so attributes can be added to
 *                       it. This should be set up by a child class
 *             log - to log any errors
 *
 *********************************************************************************************/

void Action::saveData(pugi::xml_node &entnode) const {

	// First, call the parent version
	Entity::saveData(entnode);

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

int Action::loadData(LogMgr &log, pugi::xml_node &entnode) {

	// First, call the parent function
	int results = 0;
	if ((results = Entity::loadData(log, entnode)) != 1)
		return results;

	// Now populate organism data (none yet)
   // pugi::xml_attribute attr = entnode.find_attribute("id");
   // if (attr == nullptr) {
   //    log.writeLog("Entity save file missing mandatory 'id' field.", 2);
   //    return 0;
   // }
   // _id = attr.value();

	return 1;
}

/*********************************************************************************************
 * execute - executes this action
 *
 *    Params:  
 *
 *    Returns: 1 for success, 0 for failure
 *
 *********************************************************************************************/

int Action::execute(MUD &engine) {
	std::cout << "Executing: " << getID() << std::endl;

	int results = 0;
	if (_atype == Hardcoded) {
		results = (*_act_ptr)(engine, *this);
	}
	else if (_atype == Script) {
		// TODO

	} else {
		throw std::runtime_error("Unsupported action type found.");
	}

	return results;
}

/*********************************************************************************************
 * setExecuteNow - Sets the execution time to the current time
 *
 *********************************************************************************************/

void Action::setExecuteNow()
{
	_exec_time = std::chrono::system_clock::now();
}

/*********************************************************************************************
 * setAgent - sets the organism that is taking this action
 *
 *    Params:  agent - a shared_ptr that has been dynamically-cast to an Organism pointer (using
 *							  dynamic_pointer_cast to maintain pointer tracking)
 *
 *********************************************************************************************/

void Action::setAgent(std::shared_ptr<Organism> agent) {
	_agent = agent;
}


