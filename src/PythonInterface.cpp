#include <sstream>
#include "PythonInterface.h"
#include "Organism.h"
#include "exceptions.h"
#include "global.h"
#include "Static.h"
#include "Door.h"
#include "Script.h"

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

/*********************************************************************************************
 * getScript - Given an id, gets an IScript that points to the Script Entity
 *
 * Throws: script_error if the entity is not found
 *
 *********************************************************************************************/

IScript IMUD::getScript(const char *id) {
   EntityDB &edb = *(engine.getEntityDB());
   std::shared_ptr<Script> sptr = edb.getScript(id);

   if (sptr == nullptr) {
      std::stringstream errmsg;

      errmsg << "getScript could not retrieve '" << id << "'. It may not exist.";
      throw script_error(errmsg.str().c_str());
   }

   return IScript(sptr);
}

/*********************************************************************************************
 * sendMsgAll - sends the given message to everyone in the MUD
 *	sendMsgExc - sends given message to the MUD, excluding the second parameter
 *
 * Throws: 
 *
 *********************************************************************************************/

int IMUD::sendMsgAll(const char *msg) {
	return engine.getUserMgr()->sendMsg(msg, NULL, NULL, nullptr);
}

int IMUD::sendMsgExc(const char *msg, IPhysical exclude) {
   return engine.getUserMgr()->sendMsg(msg, NULL, NULL, exclude._eptr);
}

/*********************************************************************************************
 * addScript - Copies the script and adds it to the execution queue for the MUD
 *
 * Throws:
 *
 *********************************************************************************************/

int IMUD::addScript(IScript the_script) {
	// Make a copy as this will be destroyed when complete
	std::shared_ptr<Script> new_script(new Script(*the_script._sptr));

	// Clear the old copy of things specific to this instance
	the_script._sptr->clearVariables();

	return engine.getActionMgr()->addScript(new_script);
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

IPhysical IPhysical::getCurLoc() {
	return IPhysical(_eptr->getCurLoc());
}

// For the other side of the door
IPhysical IPhysical::getCurLoc2() {
	std::shared_ptr<Door> dptr = std::dynamic_pointer_cast<Door>(_eptr);

	if (dptr == nullptr)
		throw script_error("getCurLoc2 called on non-Door object.");

   return IPhysical(dptr->getCurLoc2());
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
 *				 an Organism. Second version is for locations to exclude the actor
 *
 *********************************************************************************************/

void IPhysical::sendMsg(const char *msg) {
	std::shared_ptr<Organism> optr = std::dynamic_pointer_cast<Organism>(_eptr);

	// Send to an organism?
	if (optr != NULL) {
		optr->sendMsg(msg);
		return;
	}

	// Send to a location?
	std::shared_ptr<Location> lptr = std::dynamic_pointer_cast<Location>(_eptr);
	if (lptr != NULL) {
		lptr->sendMsg(msg);
		return;
	}

	std::stringstream errmsg;
	throw script_error("sendMsg special function called on a non-organism or location.");
}

void IPhysical::sendMsgExc(const char *msg, IPhysical exclude) {

	// This function only works for location, hence the exclude function
   std::shared_ptr<Location> lptr = std::dynamic_pointer_cast<Location>(_eptr);

   if (lptr == nullptr) {
		throw script_error("sendMsg special function with exclude called on a non-location.");
   }

   lptr->sendMsg(msg, exclude._eptr);
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

/*********************************************************************************************
 * getDoorState/setDoorState - gets and sets the door state. Options: open, closed, locked, magic
 *
 *********************************************************************************************/

std::string IPhysical::getDoorState() {
	std::shared_ptr<Static> sptr = std::dynamic_pointer_cast<Static>(_eptr);

	if (sptr == nullptr) {
		throw script_error("getDoorState special function called on nonStatic");
	}
	
	const char *doorstate[] = {"open", "closed", "locked", "special"};
	return std::string(doorstate[sptr->getDoorState()]);
}

void IPhysical::setDoorState(const char *state) {

	std::shared_ptr<Static> sptr = std::dynamic_pointer_cast<Static>(_eptr);
   if (sptr == nullptr) {
      throw script_error("setDoorState special function called on nonStatic");
   }

	if (!sptr->setDoorState(state)) {
		std::string errmsg("setDoorState special function called with improper state: ");
		errmsg += state;
      throw script_error(errmsg.c_str());
	}
}

/*********************************************************************************************
 * isContained/isContainedID - returns true if the given object is contained by the parameter
 *						ID version takes a string ID of the target object we're looking for
 *
 *********************************************************************************************/

bool IPhysical::isContained(IPhysical target) {
	return _eptr->containsPhysical(target._eptr);	
}
	


bool IPhysical::isContainedID(const char *id) {
   EntityDB &edb = *(engine.getEntityDB());
   std::shared_ptr<Physical> eptr = edb.getPhysical(id);

   if (eptr == nullptr) {
      std::stringstream errmsg;

      errmsg << "isContainedByID could not retrieve '" << id << "'. It may not exist.";
      throw script_error(errmsg.str().c_str());
   }

	return _eptr->containsPhysical(eptr);

}

/*********************************************************************************************
 * addInt/Float/StrAttribute - Adds the specified type of attribute by the given name to the
 *						physical entity
 *
 *********************************************************************************************/

bool IPhysical::addIntAttribute(const char *attr, int value) {
	return _eptr->addAttribute(attr, value);
}

bool IPhysical::addFloatAttribute(const char *attr, float value) {
   return _eptr->addAttribute(attr, value);
}

bool IPhysical::addStrAttribute(const char *attr, const char *value) {
   return _eptr->addAttribute(attr, value);
}

/*********************************************************************************************
 * hasAttribute - returns true if the attribute is attached to the physical entity, false otherwise
 *
 *********************************************************************************************/

bool IPhysical::hasAttribute(const char *attr) {
   return _eptr->hasAttribute(attr);
}


/*********************************************************************************************
 * set/clrExit - Links a location exit to a new entity or removes an exit
 *
 *********************************************************************************************/

bool IPhysical::setExit(const char *exit, IPhysical new_exit) {
   std::shared_ptr<Location> locptr = std::dynamic_pointer_cast<Location>(_eptr);

   if (locptr == nullptr) {
      throw script_error("addExit special function called on non-Location.");
   }

	if ((std::dynamic_pointer_cast<Location>(new_exit._eptr) == nullptr) && 
		 (std::dynamic_pointer_cast<Door>(new_exit._eptr) == nullptr)) {
		throw script_error("addExit new_exit parameter is not a Location or Door. Cannot set exit.");
	}

	return locptr->setExit(exit, new_exit._eptr);
}

bool IPhysical::clrExit(const char *exit) {
   std::shared_ptr<Location> locptr = std::dynamic_pointer_cast<Location>(_eptr);

   if (locptr == nullptr) {
      throw script_error("addExit special function called on non-Location.");
   }

	return locptr->clrExit(exit);
}

/*********************************************************************************************
 * operator == - compare two IPhysicals for equivalency
 *
 *********************************************************************************************/

bool IPhysical::operator == (const IPhysical &comp) {
	return (_eptr == comp._eptr);
}

bool IPhysical::operator != (const IPhysical &comp) {
   return !(_eptr == comp._eptr);
}


IScript::IScript(std::shared_ptr<Script> sptr):
                                 _sptr(sptr)
{

}

IScript::IScript(const IScript &copy_from):
                        _sptr(copy_from._sptr)
{

}

/*********************************************************************************************
 * loadVariable - adds a variable to a script so it will be available when executed
 *
 *		Params:	varname - string that the script will use to refer to the variable
 *					variable - IPhysical of the variable
 *
 *********************************************************************************************/

void IScript::loadVariable(const char *varname, IPhysical variable) {
	if (!_sptr->addVariable(varname, variable._eptr)) {
		throw script_error("Attempt to add variable to script that is already there.");		
	}
}


