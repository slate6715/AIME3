#include <sstream>
#include "PythonInterface.h"
#include "Organism.h"
#include "exceptions.h"
#include "global.h"
#include "Static.h"
#include "Door.h"
#include "Script.h"
#include "Equipment.h"

void checkNull(std::shared_ptr<Physical> ptr) {
	if (ptr == nullptr)
		throw script_error("Method's object or parameter was null but was treated as non-null.");
}

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

int IMUD::sendMsgExc(const char *msg, IPhysical &exclude) {
   return engine.getUserMgr()->sendMsg(msg, NULL, NULL, exclude._eptr);
}

/*********************************************************************************************
 * addScript - Copies the script and adds it to the execution queue for the MUD
 *
 * Throws:
 *
 *********************************************************************************************/

int IMUD::addScript(IScript &the_script) {
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
	checkNull(_eptr);

	if (_eptr->getCurLoc() == nullptr)
		return "none";

	return std::string(_eptr->getCurLoc()->getID());
}

IPhysical IPhysical::getCurLoc() {
	return IPhysical(_eptr->getCurLoc());
}

// For the other side of the door
IPhysical IPhysical::getCurLoc2() {
	checkNull(_eptr);

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
	checkNull(_eptr);

	std::string buf;
	_eptr->getGameName(buf);
	return buf;
}

/*********************************************************************************************
 * getID - Gets the unique ID of this physical
 *
 *********************************************************************************************/

std::string IPhysical::getID() {
	if (_eptr == nullptr)
		return std::string("none");

   return std::string(_eptr->getID());
}

/*********************************************************************************************
 * isFlagSet - Checks if the indicated flag is set
 *
 *********************************************************************************************/

bool IPhysical::isFlagSet(const char *flagname) {
   checkNull(_eptr);

   bool results = false;
   try {
      results = _eptr->isFlagSet(flagname);
   } catch (const std::invalid_argument &e) {
      throw script_error(e.what());
   }
   return results;
}

/*********************************************************************************************
 * sendMsg - sends the text to an Organism associated with this Physical (or does nothing if not 
 *				 an Organism. Second version is for locations to exclude the actor
 *
 *********************************************************************************************/

void IPhysical::sendMsg(const char *msg) {
   checkNull(_eptr);

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

void IPhysical::sendMsgExc(const char *msg, IPhysical &exclude) {
   checkNull(_eptr);

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

void IPhysical::moveTo(IPhysical &new_loc) {
   checkNull(_eptr);

	if (!_eptr->movePhysical(new_loc._eptr, _eptr)) {
      std::stringstream errmsg;

      errmsg << "moveTo special function failed for some reason moving: " << _eptr->getID();
      throw script_error(errmsg.str().c_str());
	}
}

/*********************************************************************************************
 * destroy - for non-clones, actually just sets their location to null
 *
 *********************************************************************************************/

void IPhysical::destroy() {
   checkNull(_eptr);

	std::shared_ptr<Physical> cur_loc = _eptr->getCurLoc();
	if (cur_loc == nullptr) {
		throw script_error("destroy function called on object with null location.");
	}

	if (!cur_loc->removePhysical(_eptr)) {
      std::stringstream errmsg;

      errmsg << "destroy special function failed for some reason with: " << _eptr->getID();
      throw script_error(errmsg.str().c_str());
   }
	_eptr = nullptr;
}

/*********************************************************************************************
 * showLocation - displays the location to the player, usually used after teleporting them
 *
 *********************************************************************************************/

void IPhysical::showLocation() {
   checkNull(_eptr);

	std::shared_ptr<Organism> optr = std::dynamic_pointer_cast<Organism>(_eptr);
	if (optr == nullptr)
		throw script_error("showLocation called on non-organism.");

	optr->sendCurLocation();
}

/*********************************************************************************************
 * damage - damages the organism, killing them if health reaches zero
 *
 *********************************************************************************************/

bool IPhysical::damage(int amount) {
   checkNull(_eptr);

	if (amount <= 0) {
		throw script_error("damage function - damage amount must be greater than zero.");
	}

	std::shared_ptr<Organism> optr = std::dynamic_pointer_cast<Organism>(_eptr);

	if (optr == nullptr) {
		throw script_error("damage function called on non-Organism.\n");
	}

	return optr->damage((unsigned int) amount);
}


/*********************************************************************************************
 * getDoorState/setDoorState - gets and sets the door state. Options: open, closed, locked, magic
 *
 *********************************************************************************************/

std::string IPhysical::getDoorState() {
   checkNull(_eptr);

	std::shared_ptr<Static> sptr = std::dynamic_pointer_cast<Static>(_eptr);

	if (sptr == nullptr) {
		throw script_error("getDoorState special function called on nonStatic");
	}
	
	const char *doorstate[] = {"open", "closed", "locked", "special"};
	return std::string(doorstate[sptr->getDoorState()]);
}

void IPhysical::setDoorState(const char *state) {
   checkNull(_eptr);

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

bool IPhysical::isContained(IPhysical &target) {
   checkNull(_eptr);
	return _eptr->containsPhysical(target._eptr);	
}
	


bool IPhysical::isContainedID(const char *id) {
   checkNull(_eptr);
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
   checkNull(_eptr);
	return _eptr->addAttribute(attr, value);
}

bool IPhysical::addFloatAttribute(const char *attr, float value) {
   checkNull(_eptr);
   return _eptr->addAttribute(attr, value);
}

bool IPhysical::addStrAttribute(const char *attr, const char *value) {
   checkNull(_eptr);
   return _eptr->addAttribute(attr, value);
}

/*********************************************************************************************
 * getInt/Float/StrAttribute - Returns the specified type of attribute by the given name to the
 *                physical entity
 *
 *********************************************************************************************/

int IPhysical::getIntAttribute(const char *attr) {
   checkNull(_eptr);
   return _eptr->getAttribInt(attr);
}

float IPhysical::getFloatAttribute(const char *attr) {
   checkNull(_eptr);
   return _eptr->getAttribFloat(attr);
}

std::string IPhysical::getStrAttribute(const char *attr) {
   checkNull(_eptr);
	std::string buf;
   return std::string(_eptr->getAttribStr(attr, buf));
}

/*********************************************************************************************
 * hasAttribute - returns true if the attribute is attached to the physical entity, false otherwise
 *
 *********************************************************************************************/

bool IPhysical::hasAttribute(const char *attr) {
   checkNull(_eptr);
   return _eptr->hasAttribute(attr);
}

/*********************************************************************************************
 * isEquipped - returns true if the attribute is equipped in the given body part
 *
 *********************************************************************************************/

bool IPhysical::isEquipped(const char *name, const char *group, IPhysical &equip_ptr) {
   checkNull(_eptr);
	if ((group == NULL) && (name == NULL)) {
		throw script_error("isEquipped body part group and name both null not supported yet.");
	}

   // Just return false if this is not equipment
   std::shared_ptr<Equipment> eqptr = std::dynamic_pointer_cast<Equipment>(equip_ptr._eptr);
   if (eqptr == nullptr)
      return false;

	std::shared_ptr<Organism> optr = std::dynamic_pointer_cast<Organism>(_eptr);
	if (optr == nullptr)
		throw script_error("isEquipped called on a non-Organism");

	return optr->findBodyPartContained(name, group, eqptr);
}

bool IPhysical::isEquippedContained(const char *name, const char *group, IContained &equip_ptr) {
   checkNull(_eptr);
   if ((group == NULL) && (name == NULL)) {
      throw script_error("isCEquipped body part group and name both null not supported yet.");
   }

	// Just return false if this is not equipment
	std::shared_ptr<Equipment> eqptr = std::dynamic_pointer_cast<Equipment>(equip_ptr._eptr);
	if (eqptr == nullptr)
		return false;

   std::shared_ptr<Organism> optr = std::dynamic_pointer_cast<Organism>(_eptr);
   if (optr == nullptr)
      throw script_error("isCEquipped called on a non-Organism");

   return optr->findBodyPartContained(name, group, eqptr);
}


/*********************************************************************************************
 * set/clrExit - Links a location exit to a new entity or removes an exit
 *
 *********************************************************************************************/

bool IPhysical::setExit(const char *exit, IPhysical &new_exit) {
   checkNull(_eptr);
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
   checkNull(_eptr);
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

IPhysical::iterator IPhysical::begin() 
{ 
   checkNull(_eptr);
	return IPhysical::iterator(_eptr->begin()); 
}

IPhysical::iterator IPhysical::end() 
{ 
   checkNull(_eptr);
	return IPhysical::iterator(_eptr->end()); 
}

IPhysical::iterator::iterator(std::list<std::shared_ptr<Physical>>::iterator ptr):
																			_iptr(ptr),
																			_cptr(nullptr)
{
}

IPhysical::iterator::iterator(const IPhysical::iterator &copy_from):
                                                         _iptr(copy_from._iptr),
                                                         _cptr(copy_from._cptr)
{
}

IPhysical::iterator IPhysical::iterator::operator++()
{ 
	_iptr++; 
	return *this; 
};

IPhysical::iterator IPhysical::iterator::operator++(int junk) 
{ 
	(void) junk; 
	iterator i = *this;
	_iptr++; 
	return i; 
};

IContained &IPhysical::iterator::operator*() 
{ 
	return (_cptr = IContained(*_iptr)); 
};

IContained *IPhysical::iterator::operator->() 
{ 
	return &(_cptr = IContained(*_iptr)); 
};

bool IPhysical::iterator::operator == (const iterator &rhs) 
{ 
	return _iptr == rhs._iptr; 
};

bool IPhysical::iterator::operator != (const iterator &rhs) 
{ 
	return _iptr != rhs._iptr; 
};


/*********************************************************************************************
 * getInt/Float/StrAttribute - Returns the specified type of attribute by the given name to the
 *                physical entity
 *
 *********************************************************************************************/

int IContained::getIntAttribute(const char *attr) {
   checkNull(_eptr);
   return _eptr->getAttribInt(attr);
}

float IContained::getFloatAttribute(const char *attr) {
   checkNull(_eptr);
   return _eptr->getAttribFloat(attr);
}

std::string IContained::getStrAttribute(const char *attr) {
   checkNull(_eptr);
	std::string buf;
   return std::string(_eptr->getAttribStr(attr, buf));
}

/*********************************************************************************************
 * getTitle - Gets the Title of this Physical (Title form varies by type)
 *
 *********************************************************************************************/

std::string IContained::getTitle() {
   checkNull(_eptr);
   std::string buf;
   _eptr->getGameName(buf);
   return buf;
}

/*********************************************************************************************
 * getID - Gets the unique ID of this physical
 *
 *********************************************************************************************/

std::string IContained::getID() {
   checkNull(_eptr);
   return std::string(_eptr->getID());
}

/*********************************************************************************************
 * isFlagSet - Checks if the indicated flag is set
 *
 *********************************************************************************************/

bool IContained::isFlagSet(const char *flagname) {
   checkNull(_eptr);

	bool results = false;
	try {
		results = _eptr->isFlagSet(flagname);
	} catch (const std::invalid_argument &e) {
		throw script_error(e.what());
	}
	return results;
}

/*********************************************************************************************
 * hasAttribute - returns true if the attribute is attached to the physical entity, false otherwise
 *
 *********************************************************************************************/

bool IContained::hasAttribute(const char *attr) {
   checkNull(_eptr);
   return _eptr->hasAttribute(attr);
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

void IScript::loadVariable(const char *varname, IPhysical &variable) {
	if (!_sptr->addVariable(varname, variable._eptr)) {
		throw script_error("Attempt to add variable to script that is already there.");		
	}
}

/*********************************************************************************************
 * setInterval - changes the interval of this script
 *
 *    Params:  varname - string that the script will use to refer to the variable
 *
 *********************************************************************************************/

void IScript::setInterval(float interval) {
	if (interval < 0) {
		throw script_error("Attempt to set interval to a value < 0");
	}
	_sptr->setInterval(interval);
}

