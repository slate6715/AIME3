#include <cstring>
#include <iostream>
#include <sstream>
#include <regex>
#include "Physical.h"
#include "global.h"
#include "Attribute.h"
#include "misc.h"
#include "ScriptEngine.h"
#include "Static.h"

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
 *    Params:  entnode - This physical's node within the XML tree so attributes can be drawn from
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
 * addPhysical - adds the specified physical to the container
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
 * removePhysical - Removes the physical from the container
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
 * movePhysical - Moves this physical to a new location, first removing it from the old locale and
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
 * getContainedByPtr - retrieves the shared pointer of the contained physical based on a regular pointer
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
 * getContainedByID - returns a shared_ptr to the contained physical that matches name_alias. This
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
 * getContainedByName - returns a shared_ptr to the contained physical that matches the name.
 *					Polymorphic. For this class version, only matches the game name field which varies
 *             depending on the physical class
 *
 *		Params:	name - string to search the NameID for
 *					allow_abbrev - if true, physical only needs to match up to sizeof(name)
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
		lower(ebuf);

      if ((!allow_abbrev) && (namebuf.compare(ebuf) == 0))
         return *eit;
		else if ((allow_abbrev) && equalAbbrev(namebuf, ebuf.c_str()))
			return *eit;

		// See if the game name starts with "the " and try without it
		if (ebuf.find("the ") != std::string::npos) {
			std::string nothe = ebuf.substr(4, ebuf.size()-4);

	      if ((!allow_abbrev) && (namebuf.compare(nothe) == 0))
		      return *eit;
			else if ((allow_abbrev) && equalAbbrev(namebuf, nothe.c_str()))
				return *eit;

		}
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
 * containsLit - searches the contents of this Physical for anything with the Static::Lit flag.
 *
 *		Params:	recursive_lvl - how far deep to call recursive searches (INT_MAX for unlimited)
 *
 *		Returns:	pointer to the lit object if found, nullptr otherwise
 *
 *********************************************************************************************/

std::shared_ptr<Physical> Physical::containsLit(int recursive_lvl) {
	if (recursive_lvl < 0)
		return nullptr;

	std::shared_ptr<Physical> use_item;
	std::shared_ptr<Static> sptr;

	auto phyit = _contained.begin();
	for ( ; phyit != _contained.end(); phyit++) {
		if ((use_item = (*phyit)->containsLit(recursive_lvl-1)) != nullptr)
			return use_item;

		if ((sptr = std::dynamic_pointer_cast<Static>(*phyit)) == nullptr)
			continue;

		if (sptr->isStaticFlagSet(Static::Lit))
			return sptr;	
	}
	return nullptr;
}

/*********************************************************************************************
 * containsFlag - searches the contents of this Physical for anything with the indicated flag set.
 *
 *    Params:  recursive_lvl - how far deep to call recursive searches (INT_MAX for unlimited)
 *
 *		Returns:	pointer to the flagged object if found, nullptr otherwise
 *
 *********************************************************************************************/

std::shared_ptr<Physical> Physical::containsFlag(const char *flagname, int recursive_lvl) {
   if (recursive_lvl < 0)
      return nullptr;

	std::shared_ptr<Physical> use_item;

	std::string flagstr = flagname;
	lower(flagstr);

   auto phyit = _contained.begin();
   for ( ; phyit != _contained.end(); phyit++) {
      if ((use_item = (*phyit)->containsFlag(flagstr.c_str(), recursive_lvl-1)) != nullptr)
         return use_item;

		// Check if the flag is set, but catch invalid_argument exceptions if flag doesn't exist
		try {
			if ((*phyit)->isFlagSet(flagstr.c_str()))
				return *phyit;
		} catch (std::invalid_argument &e) {
			continue;
		}
   }
   return nullptr;

}


/*********************************************************************************************
 * containsFlags - searches the contents of this Physical for anything with all the flags listed set
 *
 *    Params:	flags - vector of flag strings - must be lowercase
 *					recursive_lvl - how far deep to call recursive searches (INT_MAX for unlimited)
 *
 *    Returns: pointer to the flagged object if found, nullptr otherwise
 *
 *********************************************************************************************/

std::shared_ptr<Physical> Physical::containsFlags(std::vector<std::string> &flags, int recursive_lvl) {
   if (recursive_lvl < 0)
      return nullptr;

   std::shared_ptr<Physical> use_item;

   auto phyit = _contained.begin();
   for ( ; phyit != _contained.end(); phyit++) {
      if ((use_item = (*phyit)->containsFlags(flags, recursive_lvl-1)) != nullptr)
         return use_item;

      // Check if the flag is set, but catch invalid_argument exceptions if flag doesn't exist
		unsigned int i;
		for (i=0; i<flags.size(); i++) {
			try {
				if (!(*phyit)->isFlagSet(flags[i].c_str()))
					break;	
			} catch (std::invalid_argument &e) {
				break;
			}
		}
		if (i == flags.size())
			return *phyit;
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
 * isFlagSetInternal - given the flag string, returns the flag setting for this physical
 *                      Internal version calls up to parphyss to check their flags.
 *
 *    Returns: true if flag is set, false otherwise
 *
 *    Throws: invalid_argument if the flag is not a valid flag
 *
 *********************************************************************************************/

bool Physical::isFlagSetInternal(const char *flagname, bool &results) {
	// No physical flags yet
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
 * addAttribute, remAttribute - add and remove attributes from the list
 *
 *		Params: attrib - id for the new attribute or one to be removed
 *				  value - initial value - must be a decimal (e.g. 1.0)  to add a float attribute
 *		
 *    Returns: true if added/removed, false if already there (for add) or not found (for rem)
 *
 *********************************************************************************************/

bool Physical::addAttribute(const char *attrib, int value) {
	std::shared_ptr<Attribute> new_attr(new IntAttribute(value));

   auto m_it = _attributes.insert(std::pair<std::string, std::shared_ptr<Attribute>>(attrib, new_attr));

	return m_it.second;
}

bool Physical::addAttribute(const char *attrib, float value) {
   std::shared_ptr<Attribute> new_attr(new FloatAttribute(value));

   auto m_it = _attributes.insert(std::pair<std::string, std::shared_ptr<Attribute>>(attrib, new_attr));

   return m_it.second;
}

bool Physical::addAttribute(const char *attrib, const char *value) {
   std::shared_ptr<Attribute> new_attr(new StrAttribute(value));

   auto m_it = _attributes.insert(std::pair<std::string, std::shared_ptr<Attribute>>(attrib, new_attr));

   return m_it.second;
}

bool Physical::remAttribute(const char *attrib) {
	std::string attrstr(attrib);
	return _attributes.erase(attrstr);	
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
	auto m_it = _attributes.find(attrib);
	if (m_it == _attributes.end())
		return false;

	*(m_it->second) = value;
	return true;
}

bool Physical::setAttribute(const char *attrib, float value) {
   auto m_it = _attributes.find(attrib);
   if (m_it == _attributes.end())
      return false;

   *(m_it->second) = value;
   return true;
}

// Special function also allows for string int and floats (converts internally)
bool Physical::setAttribute(const char *attrib, const char *value) {
   auto m_it = _attributes.find(attrib);
   if (m_it == _attributes.end())
      return false;

   *(m_it->second) = value;
   return true;
}

// Special function also allows for string int and floats (converts internally)
bool Physical::setAttribute(const char *attrib, Attribute &value) {
   auto m_it = _attributes.find(attrib);
   if (m_it == _attributes.end())
      return false;

   *(m_it->second) = value;
   return true;
}

int Physical::getAttribInt(const char *attrib) {
   auto m_it = _attributes.find(attrib);
   if (m_it == _attributes.end()) {
		throw std::invalid_argument("Request for attribute that doesn't exist.\n");
	}
   return m_it->second->getInt(); 
}

float Physical::getAttribFloat(const char *attrib) {
   auto m_it = _attributes.find(attrib);
   if (m_it == _attributes.end()) {
      throw std::invalid_argument("Request for attribute that doesn't exist.\n");
   }
   return m_it->second->getFloat();
}


const char *Physical::getAttribStr(const char *attrib, std::string &buf) {
   auto m_it = _attributes.find(attrib);
   if (m_it == _attributes.end()) {
      throw std::invalid_argument("Request for attribute that doesn't exist.\n");
   }
   buf = m_it->second->getStr();
	return buf.c_str();
}

/*********************************************************************************************
 * hasAttribute/hasAttributeInternal - simple boolean check to see if an attribute exists
 *
 *    Returns: true if found and set, false otherwise
 *
 *********************************************************************************************/

bool Physical::hasAttribute(const char *attrib) {
   auto m_it = _attributes.find(attrib);
   if (m_it == _attributes.end()) {
		return false;
   }
	return true;
}

/*********************************************************************************************
 * getAttribType - locates the attribute by its string and returns the type.
 *                   calls a polymorphic version getAttribTypeInternal
 *
 *    Returns: attr_type of Int, Float, Str, or Undefined if not found
 *
 *********************************************************************************************/

Attribute::attr_type Physical::getAttribType(const char *attrib) const {
   auto m_it = _attributes.find(attrib);
   if (m_it == _attributes.end()) {
      throw std::invalid_argument("Request for attribute that doesn't exist.\n");
   }

	return m_it->second->getType();
}

/*********************************************************************************************
 * fillAttrXMLNode - populates the parameter XML node with data from this physical's attributes. 
 *                   polymorphic
 *
 *    Returns: true if the same, false otherwise
 *
 *********************************************************************************************/

void Physical::fillAttrXMLNode(pugi::xml_node &anode) const {

   // Populate with all the attributes
   pugi::xml_node nextnode;
   pugi::xml_attribute nextattr;

	auto m_it = _attributes.begin();
	for ( ; m_it != _attributes.end(); m_it++) {
      nextnode = anode.append_child("attribute");
      nextattr = nextnode.append_attribute("name");
      nextattr.set_value(m_it->first.c_str());
		m_it->second->fillXMLNode(nextnode);
   }
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
 * execSpecial - looks for the given trigger attached to this physical and executes it if found.
 *               
 *		Params:	trigger - the trigger string to match
 *					variables - variables to use in the special, like actor, target1, etc
 *
 *		Returns: -1 for error
 *					0 indicates the trigger was not found attached to this physical
 *					1 indicates the trigger executed and the command process should proceed normally
 *					2 indicates the trigger executed and the command process should terminate
 *
 *********************************************************************************************/

int Physical::execSpecial(const char *trigger, 
								std::vector<std::pair<std::string, std::shared_ptr<Physical>>> &variables) {

	ScriptEngine &se = *engine.getScriptEngine();

	// Loop through all the specials attached to this object
	for (unsigned int i=0; i<_specials.size(); i++) {
		
		// Compare special against trigger
		if (_specials[i].first.compare(trigger) == 0)
		{
			for (unsigned int i=0; i<variables.size(); i++)
				se.setVariable(variables[i].first.c_str(), variables[i].second);

			se.setVariable("this", std::dynamic_pointer_cast<Physical>(_self));

			int results = se.execute(_specials[i].second);
			if (results == 1)
				return 2;

			return 1;
		}
	}

	// No special found with that trigger, exit
	return 0;
}


