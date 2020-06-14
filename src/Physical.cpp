#include <cstring>
#include <iostream>
#include <sstream>
#include <regex>
#include "Physical.h"
#include "global.h"
#include "Attribute.h"
#include "misc.h"
#include "ScriptEngine.h"

/*********************************************************************************************
 * PhysicalDB (constructor) - Called by a child class to initialize any Physical elemphyss
 *
 *********************************************************************************************/
Physical::Physical(const char *id):
								Entity(id)
{


}

// Called by child class
Physical::Physical(const Physical &copy_from):
										Entity(copy_from)
{

}


Physical::~Physical() {

}

/*********************************************************************************************
 * saveData - Called by a child class to save Physical-specific data into an XML documphys
 *
 *		Params:	entnode - This Entity's node within the XML tree so attributes can be added to
 *                       it. This should be set up by a child class
 *
 *********************************************************************************************/

void Physical::saveData(pugi::xml_node &entnode) const {

	Entity::saveData(entnode);

	// Save attributes (polymorphic)
	fillAttrXMLNode(entnode);
}

/*********************************************************************************************
 * loadData - Called by a child class to populate Physical-specific data from an XML documphys
 *
 *    Params:  entnode - This physity's node within the XML tree so attributes can be drawn from
 *								 it
 *
 *		Returns: 1 for success, 0 for failure
 *
 *********************************************************************************************/

int Physical::loadData(pugi::xml_node &entnode) {
	std::stringstream errmsg;
	
	if (!Entity::loadData(entnode))
		return 0;

   for (pugi::xml_node flag = entnode.child("attribute"); flag; flag = flag.next_sibling("attribute")) {
      try {
         pugi::xml_attribute attr = flag.attribute("name");
         if (attr == nullptr) {
            errmsg << getTypeName() << " '" << getID() << "' attribute node missing mandatory name field.";
            mudlog->writeLog(errmsg.str().c_str());
            return 0;
         }
			std::string name = attr.value();

			attr = flag.attribute("value");
         if (attr == nullptr) {
            errmsg << getTypeName() << " '" << getID() << "' attribute node missing mandatory value field.";
            mudlog->writeLog(errmsg.str().c_str());
            return 0;
         }
			setAttribute(name.c_str(), attr.value());
      }
      catch (std::invalid_argument &e) {
         errmsg << getTypeName() << " '" << getID() << "' attribute error: " << e.what();
         mudlog->writeLog(errmsg.str().c_str());
         return 0;
      }
   }

	for (pugi::xml_node special = entnode.child("special"); special; special = special.next_sibling("special")) {
		try {
			pugi::xml_attribute attr = special.attribute("trigger");
         if (attr == nullptr) {
            errmsg << getTypeName() << " '" << getID() << "' special node missing mandatory trigger field.";
            mudlog->writeLog(errmsg.str().c_str());
            return 0;
         }
         std::string trigger = attr.value();

			_specials.push_back(std::pair<std::string, std::string>(trigger, special.child_value()));
		}
      catch (std::invalid_argument &e) {
         errmsg << getTypeName() << " '" << getID() << "' specials error: " << e.what();
         mudlog->writeLog(errmsg.str().c_str());
         return 0;
      }
	}

	return 1;

}

/*********************************************************************************************
 * containsPhysical - Checks if the physical is contained within this physical's container
 *
 *    Returns: true for contained, false otherwise
 *
 *********************************************************************************************/

bool Physical::containsPhysical(std::shared_ptr<Physical> phys_ptr) {
	auto cptr = _contained.begin();
	for ( ; cptr != _contained.end(); cptr++) {
		if (phys_ptr == *cptr)
			return true;
	}
	return false;
}

/*********************************************************************************************
 * addPhysical - adds the specified physity to the container
 *
 *    Returns: true if added, false if it was already contained
 *
 *********************************************************************************************/

bool Physical::addPhysical(std::shared_ptr<Physical> new_phys) {
	if (containsPhysical(new_phys))
		return false;

	_contained.push_back(new_phys);
	return true;
}

/*********************************************************************************************
 * removePhysical - Removes the physity from the container
 *
 *    Returns: true if found and removed, false if it was not found
 *
 *********************************************************************************************/

bool Physical::removePhysical(std::shared_ptr<Physical> phys_ptr) {
	auto cptr = _contained.begin();

   for ( ; cptr != _contained.end(); cptr++) {
      if (phys_ptr == *cptr){
         _contained.erase(cptr);
			phys_ptr->_cur_loc = nullptr;
			return true;
		}
   }
   return false;
}

/*********************************************************************************************
 * movePhysical - Moves this physity to a new location, first removing it from the old locale and
 *					 planting it in the new one
 *
 *		Params:	new_phys - the new location for this object
 *					self - shared_ptr of itself - if null, will look in the _cur_loc for its shared
 *							 pointer
 *
 *    Returns: true if successful, false if there was an anomaly
 *
 *********************************************************************************************/

bool Physical::movePhysical(std::shared_ptr<Physical> new_phys, std::shared_ptr<Physical> self) {
	bool results = true;

	if (new_phys == nullptr) {
		throw std::runtime_error("Physical::movePhysical: attempted to set location to null Physical");
	}

	if ((_cur_loc != nullptr) && (self == nullptr)) {
		self = _cur_loc->getContainedByPtr(this);
		if (self == nullptr){
			throw std::runtime_error("Physical::movePhysical: Could not retrieve self shared_ptr");
		}
	}
	if (_cur_loc != nullptr)
		results &= _cur_loc->removePhysical(self);

	_cur_loc = new_phys;

	results &= new_phys->addPhysical(self);
	return results;
}

/*********************************************************************************************
 * getContainedByPtr - retrieves the shared pointer of the contained physity based on a regular pointer
 *
 *    Returns: applicable shared pointer (which points to null if failed) 
 *
 *********************************************************************************************/

std::shared_ptr<Physical> Physical::getContainedByPtr(Physical *eptr) {
   auto cptr = _contained.begin();

   for ( ; cptr != _contained.end(); cptr++) {
      if (eptr == &(**cptr)){
			return *cptr;
      }
   }
   return std::shared_ptr<Physical>(nullptr);
}

/*********************************************************************************************
 * getContainedByID - returns a shared_ptr to the contained physity that matches name_alias. This
 *                   version only checks id
 *
 *    Returns: shared_ptr if found, nullptr if not
 *
 *********************************************************************************************/

std::shared_ptr<Physical> Physical::getContainedByID(const char *name_alias) {
   std::string name = name_alias;

   // Search through the contained for the object
   auto eit = _contained.begin();
   for ( ; eit != _contained.end(); eit++) {

      if (name.compare(eit->get()->getID()) == 0)
         return *eit;
   }
   return nullptr;

}

/*********************************************************************************************
 * getContainedByName - returns a shared_ptr to the contained physity that matches the name.
 *					Polymorphic. For this class version, only matches the game name field which varies
 *             depending on the physity class
 *
 *		Params:	name - string to search the NameID for
 *					allow_abbrev - if true, physity only needs to match up to sizeof(name)
 *
 *    Returns: shared_ptr if found, nullptr if not
 *
 *********************************************************************************************/

std::shared_ptr<Physical> Physical::getContainedByName(const char *name, bool allow_abbrev) {
   std::string namebuf = name;
	std::string ebuf;

   // Search through the contained for the object
   auto eit = _contained.begin();
   for ( ; eit != _contained.end(); eit++) {

		// Get the Game name which might be Title or NameID
		eit->get()->getGameName(ebuf);
      if ((!allow_abbrev) && (namebuf.compare(ebuf) == 0))
         return *eit;
		else if ((allow_abbrev) && equalAbbrev(namebuf, ebuf.c_str()))
			return *eit;
   }

   // Search by altnames next
   eit = _contained.begin();
   for ( ; eit != _contained.end(); eit++) {
      if (eit->get()->hasAltName(name, allow_abbrev))
         return *eit;
   }

   return nullptr;

}

/*********************************************************************************************
 * setFlagInternal - given the flag string, sets the flag or returns false if not found.
 *								Internal version calls up to parphyss to check their flags.
 *
 *		Throws: invalid_argument if the flag is not a valid flag
 *
 *********************************************************************************************/

bool Physical::setFlagInternal(const char *flagname, bool newval) {
	// Does nothing right now
	(void) flagname;
	(void) newval;
	return false;
}

/*********************************************************************************************
 * isFlagSetInternal - given the flag string, returns the flag setting for this physity
 *                      Internal version calls up to parphyss to check their flags.
 *
 *    Returns: true if flag is set, false otherwise
 *
 *    Throws: invalid_argument if the flag is not a valid flag
 *
 *********************************************************************************************/

bool Physical::isFlagSetInternal(const char *flagname, bool &results) {
	// No physity flags yet
	(void) flagname; 
	(void) results;
	return false;
}


/*********************************************************************************************
 * purgePhysical - Removes all references to the parameter from the Entities in the database so
 *               it can be safely removed
 *
 *    Returns: number of references to this object cleared
 *
 *********************************************************************************************/

size_t Physical::purgePhysical(std::shared_ptr<Physical> item) {
	size_t count = 0;
	auto c_it = _contained.begin();

	// Look for contained items and remove it
	while (c_it != _contained.end()) {
		if ((*c_it) == item) {
			c_it = _contained.erase(c_it);
			count++;
		} else
			c_it++;
	}
	return count;
}

/*********************************************************************************************
 *	setAttribute, getAttribute - Setting and getting attribute values. These are less-efficiphys 
 *						versions of the class versions that use enum index as they have to lookup 
 *						string to enums
 *
 *
 *    Returns: true if found and set, false otherwise
 *
 *********************************************************************************************/
bool Physical::setAttribute(const char *attrib, int value) {
	return setAttribInternal(attrib, value);	
}

bool Physical::setAttribute(const char *attrib, float value) {
   return setAttribInternal(attrib, value);
}

// Special function also allows for string int and floats (converts internally)
bool Physical::setAttribute(const char *attrib, const char *value) {
   return setAttribInternal(attrib, value);
}

// Special function also allows for string int and floats (converts internally)
bool Physical::setAttribute(const char *attrib, Attribute &value) {
   return setAttribInternal(attrib, value);
}

int Physical::getAttribInt(const char *attrib) {
	int value;
	if (!getAttribInternal(attrib, value)) {
		std::stringstream msg;
		msg << "Unknown attribute requested '" << attrib << "' in Physical '" << getID() << "'";
		throw std::invalid_argument(msg.str().c_str());
	}
	return value;
}

float Physical::getAttribFloat(const char *attrib) {
   float value;
   if (!getAttribInternal(attrib, value)) {
      std::stringstream msg;
      msg << "Unknown attribute requested '" << attrib << "' in Physical '" << getID() << "'";
      throw std::invalid_argument(msg.str().c_str());
   }
   return value;
}


const char *Physical::getAttribStr(const char *attrib, std::string &buf) {
   if (!getAttribInternal(attrib, buf)) {
      std::stringstream msg;
      msg << "Unknown attribute requested '" << attrib << "' in Physical '" << getID() << "'";
      throw std::invalid_argument(msg.str().c_str());
   }
   return buf.c_str();
}

/*********************************************************************************************          
 * set/getAttributeInternal - polymorphic version of the get/setAttribute function
 *
 *    Returns: true if found and set, false otherwise
 *
 *********************************************************************************************/        
bool Physical::setAttribInternal(const char *attrib, int value) {
	(void) attrib;
	(void) value;
	return false;
}

bool Physical::setAttribInternal(const char *attrib, float value) {
   (void) attrib;
   (void) value;
   return false;
}

bool Physical::setAttribInternal(const char *attrib, const char *value) {
   (void) attrib;
   (void) value;
   return false;
}

bool Physical::setAttribInternal(const char *attrib, Attribute &value) {
   (void) attrib;
   (void) value;
   return false;
}

bool Physical::getAttribInternal(const char *attrib, int &value) {
   (void) attrib;
   (void) value;
   return false;
}

bool Physical::getAttribInternal(const char *attrib, float &value) {
   (void) attrib;
   (void) value;
   return false;
}

bool Physical::getAttribInternal(const char *attrib, std::string &value) {
   (void) attrib;
   (void) value;
   return false;
}

/*********************************************************************************************
 * getAttribType - locates the attribute by its string and returns the type.
 *                   calls a polymorphic version getAttribTypeInternal
 *
 *    Returns: attr_type of Int, Float, Str, or Undefined if not found
 *
 *********************************************************************************************/

Attribute::attr_type Physical::getAttribType(const char *attrib) const {
   return getAttribTypeInternal(attrib);
}

Attribute::attr_type Physical::getAttribTypeInternal(const char *attrib) const {
	(void) attrib;

   // No Physical-level attributes yet
   return Attribute::Undefined;
}


/*********************************************************************************************
 * fillAttrXMLNode - populates the parameter XML node with data from this physity's attributes. 
 *                   polymorphic
 *
 *    Returns: true if the same, false otherwise
 *
 *********************************************************************************************/

void Physical::fillAttrXMLNode(pugi::xml_node &anode) const {
	// Currphysly nothing in the Physical parphys class but might add cur_loc and contained
	(void) anode;	
}


/*********************************************************************************************
 * open, close - open or close a container or door - this is the default function for objects
 *				     that cannot be opened or closed. 
 *
 *********************************************************************************************/

bool Physical::open(std::string &errmsg) {
	errmsg = "You can't open that.\n";
	return false;
}

bool Physical::close(std::string &errmsg) {
	errmsg = "You can't close that.\n";
	return false;
}

/*********************************************************************************************
 * execSpecial - looks for the given trigger attached to this physity and executes it if found.
 *               
 *		Params:	trigger - the trigger string to match
 *					actor - the organism executing this action
 *
 *		Returns: -1 for error
 *					0 indicates the trigger was not found attached to this physity
 *					1 indicates the trigger executed and the command process should proceed normally
 *					2 indicates the trigger executed and the command process should terminate
 *
 *********************************************************************************************/

int Physical::execSpecial(const char *trigger, std::shared_ptr<Organism> actor) {

	// Loop through all the specials attached to this object
	for (unsigned int i=0; i<_specials.size(); i++) {
		
		// Compare special against trigger
		if (_specials[i].first.compare(trigger) == 0)
		{
			ScriptEngine script(_specials[i].second);

			script.setActor(actor);

			script.execute();

			return 1;
		}
	}

	// No special found with that trigger, exit
	return 0;
}

