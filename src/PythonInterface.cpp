#include <sstream>
#include "PythonInterface.h"
#include "Organism.h"
#include "exceptions.h"
#include "global.h"

IMUD::IMUD() {

}

IMUD::~IMUD() {

}

/*********************************************************************************************
 * getPhysical - Given an id, gets an IPhysical that points to the Physical
 *
 *	Throws: script_error if the entity is not found
 *
 *********************************************************************************************/

IPhysical IMUD::getPhysical(const char *id) {
	EntityDB &edb = *(engine.getEntityDB());
	std::shared_ptr<Physical> eptr = edb.getPhysical(id);

	if (eptr == nullptr) {
		std::stringstream errmsg;

		errmsg << "getPhysical could not retrieve '" << id << "'. It may not exist.";
		throw script_error(errmsg.str().c_str());
	}

	return IPhysical(eptr);
}

IPhysical::IPhysical(std::shared_ptr<Physical> eptr):
											_eptr(eptr)
{

}

IPhysical::IPhysical(const IPhysical &copy_from):
								_eptr(copy_from._eptr)
{

}

/*********************************************************************************************
 * getCurLocID - Gets the id string of this Physical's current location
 *
 *********************************************************************************************/

std::string IPhysical::getCurLocID() {
	if (_eptr->getCurLoc() == nullptr)
		return "none";

	return std::string(_eptr->getCurLoc()->getID());
}

/*********************************************************************************************
 * getTitle - Gets the Title of this Physical (Title form varies by type)
 *
 *********************************************************************************************/

std::string IPhysical::getTitle() {
	std::string buf;
	_eptr->getGameName(buf);
	return buf;
}

/*********************************************************************************************
 * sendMsg - sends the text to an Organism associated with this Physical (or does nothing if not 
 *				 an Organism
 *
 *********************************************************************************************/

void IPhysical::sendMsg(const char *msg) {
	std::shared_ptr<Organism> optr = std::dynamic_pointer_cast<Organism>(_eptr);

	// Fail quietly for now if this isn't the right type of entity
	if (optr == NULL)
		return;

   optr->sendMsg(msg);
}

/*********************************************************************************************
 * sendMsgLoc - sends a message to this object's Location, excluding the object if it is a
 *					 Player.
 *
 *********************************************************************************************/

void IPhysical::sendMsgLoc(const char *msg) {
   std::shared_ptr<Player> pptr = std::dynamic_pointer_cast<Player>(_eptr);

	_eptr->getCurLoc()->sendMsg(msg, pptr);
}

/*********************************************************************************************
 * moveTo - moves this entity into the container of the parameter entity
 *
 *********************************************************************************************/

void IPhysical::moveTo(IPhysical new_loc) {
	if (!_eptr->movePhysical(new_loc._eptr, _eptr)) {
      std::stringstream errmsg;

      errmsg << "moveTo special function failed for some reason moving: " << _eptr->getID();
      throw script_error(errmsg.str().c_str());
	}
}

