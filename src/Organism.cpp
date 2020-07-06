#include <sstream>
#include <climits>
#include "Organism.h"
#include "global.h"
#include "misc.h"
#include "Location.h"
#include "Attribute.h"
#include "Equipment.h"
#include "Getable.h"
#include "Trait.h"

const char *reviewlist[] = {"standing", "entering", "leaving", NULL};

const char *dir1_list[] = {"north", "south", "east", "west", "up", "down", "northeast", "northwest", 
									"southeast", "southwest", "custom", NULL};
const char *dir2_list[] = {"the north", "the south", "the east", "the west", "up", "down", "the northeast", 
								  "the northwest", "the southeast", "the southwest", "custom", NULL}; 
const char *dir3_list[] = {"the north", "the south", "the east", "the west", "above", "below", "the northeast",
                          "the northwest", "the southeast", "the southwest", "custom", NULL};
const char *org_attriblist[] = {"strength", "constitution", "dexterity", "intelligence", "wisdom", "charisma", "experience", "damage", NULL};

const char *oflag_list[] = { "NoSummon", NULL};

/*********************************************************************************************
 * Organism (constructor) - Called by a child class to initialize any Organism elements
 *
 *********************************************************************************************/
Organism::Organism(const char *id):
								Physical(id) 
{
	// Review defaults
	_reviews.push_back("%n is standing here.");				// Standing
	_reviews.push_back("%n enters the room from %3");	// Entering
	_reviews.push_back("%n departs the room %1.");		// Leaving

	// Hardcoded body parts for now, but later on, could customize for race/class
	addBodyPart("head", "head");
	addBodyPart("head", "face");
	addBodyPart("head", "neck");
	addBodyPart("torso", "chest");
	addBodyPart("torso", "back");
	addBodyPart("torso", "waist");
	addBodyPart("leftleg", "thigh");
	addBodyPart("leftleg", "foot");
	addBodyPart("rightleg", "thigh");
	addBodyPart("rightleg", "foot");
	addBodyPart("rightarm", "shoulder");
	addBodyPart("leftarm", "shoulder");
	addBodyPart("rightarm", "forearm");
	addBodyPart("leftarm", "forearm");
	addBodyPart("rightarm", "hand");
	addBodyPart("leftarm", "hand");

	setBodyPartFlag("rightarm", "hand", CanWield, true);
	setBodyPartFlag("leftarm", "hand", CanWield, true);

	// Set up our attribute list (Strength, Constitution, Dexterity, Intelligence, Wisdom, Charisma, Experience, Damage)
	addAttribute("strength", 0);
	addAttribute("constitution", 0);
	addAttribute("dexterity", 0);
	addAttribute("intelligence", 0);
	addAttribute("wisdom", 0);
	addAttribute("charisma", 0);
	addAttribute("experience", 0);
	addAttribute("damage", 0);
	addAttribute("health", 100);

	// Add our review formatter entries
	_rformatter.addMap('N', "temp");	// Name (title)
	_rformatter.addMap('n', "temp"); // Name minus the or a
	_rformatter.addMap('1', "temp"); // Exit format 1
	_rformatter.addMap('2', "temp");	// Exit format 2
	_rformatter.addMap('3', "temp");	// Exit format 3


}

// Called by child class
Organism::Organism(const Organism &copy_from):
								Physical(copy_from)
{

}


Organism::~Organism() {

}

/*********************************************************************************************
 * saveData - Called by a child class to save Organism-specific data into an XML document
 *
 *    Params:  entnode - This entity's node within the XML tree so attributes can be added to
 *                       it. This should be set up by a child class
 *             log - to log any errors
 *
 *********************************************************************************************/

void Organism::saveData(pugi::xml_node &entnode) const {

	// First, call the parent version
	Physical::saveData(entnode);

   pugi::xml_node xnode;
   pugi::xml_attribute xattr;

	// Save the examine 
	xnode = entnode.append_child("examine");
	xnode.append_child(pugi::node_pcdata).set_value(_examine.c_str());

	// Save the reviews
	const char *reviewtype[] = {"standing", "entering", "leaving", NULL};

	unsigned int i;
	for (i = 0; i<_reviews.size(); i++) {
		xnode = entnode.append_child("reviewmsg");
		xnode.append_child(pugi::node_pcdata).set_value(_reviews[i].c_str());

		xattr = xnode.append_attribute("type");
		xattr.set_value(reviewtype[i]);
	}

   // Save the traits 
   for (i=0; i<_traits.size(); i++) {
      xnode = entnode.append_child("trait");
      xattr = xnode.append_attribute("id");
      xattr.set_value(_traits[i]->getID());
   }

}

/*********************************************************************************************
 * loadData - Called by a child class to populate Organism-specific data from an XML document
 *
 *    Params:  entnode - This entity's node within the XML tree so attributes can be drawn from
 *                       it
 *
 *    Returns: 1 for success, 0 for failure
 *
 *********************************************************************************************/

int Organism::loadData(pugi::xml_node &entnode) {
	std::stringstream errmsg;

	// First, call the parent function
	int results = 0;
	if ((results = Physical::loadData(entnode)) != 1)
		return results;

	// Now populate organism data (none yet)
   pugi::xml_node node = entnode.child("examine");
   if (node == nullptr) {
      mudlog->writeLog("Organism save file missing mandatory 'examine' field.", 2);
      return 0;
   }
   _examine = node.child_value();

   // Now populate organism data (none yet)
   pugi::xml_attribute attr = entnode.attribute("title");
   if (node != nullptr) {
		_title = attr.value();
		
   }

   for (pugi::xml_node review = entnode.child("reviewmsg"); review; review = 
																					review.next_sibling("reviewmsg")) {
      try {
         attr = review.attribute("type");
         if (attr == nullptr) {
            errmsg << getTypeName() << " '" << getID() << "' reviewmsg node missing mandatory type field.";
            mudlog->writeLog(errmsg.str().c_str());
            return 0;
         }
         std::string reviewtype = attr.value();
			if (reviewtype.compare("standing") == 0) {
				setReview(Standing, review.child_value());
			} else if (reviewtype.compare("entering") == 0)
				setReview(Entering, review.child_value());
			else if (reviewtype.compare("leaving") == 0) 
				setReview(Leaving, review.child_value());
			else {
				errmsg << getTypeName() << " '" << getID() << "' reviewmsg type not recognized.";
				mudlog->writeLog(errmsg.str().c_str());
				return 0;
			}
      }
      catch (std::invalid_argument &e) {
         errmsg << getTypeName() << " '" << getID() << "' reviewmsg error: " << e.what();
         mudlog->writeLog(errmsg.str().c_str());
         return 0;
      }
	}

   for (pugi::xml_node trait = entnode.child("trait"); trait; trait =
                                                               trait.next_sibling("trait")) {
      try {
         attr = trait.attribute("id");
         if (attr == nullptr) {
            errmsg << getTypeName() << "Organism '" << getID() << "' trait node missing mandatory id field.";
            mudlog->writeLog(errmsg.str().c_str());
            return 0;
         }

			EntityDB *edb = engine.getEntityDB();

			std::shared_ptr<Trait> trait = edb->getTrait(attr.value());
			if (trait == nullptr) {
            errmsg << getTypeName() << " '" << getID() << "' trait '" << attr.value() << 
																					"' does not appear to be a valid trait.";
            mudlog->writeLog(errmsg.str().c_str());
            return 0;
			}

			addTrait(trait);
			
      }
      catch (std::invalid_argument &e) {
         errmsg << getTypeName() << " '" << getID() << "' reviewmsg error: " << e.what();
         mudlog->writeLog(errmsg.str().c_str());
         return 0;
      }
   }

	// Set up our review information
	std::string buf;
	getGameName(buf);
   _rformatter.changeMap('N', buf.c_str());
   _rformatter.changeMap('n', buf.c_str());

   size_t pos = buf.find(" ");
   if (pos != std::string::npos) {
      std::string prefix = buf.substr(0, pos);
      lower(prefix);
      if ((prefix.compare("the") == 0) || (prefix.compare("a") == 0))
         _rformatter.changeMap('n', buf.substr(pos+1, buf.size()-pos).c_str());
   }

	return 1;
}

/*********************************************************************************************
 * setFlagInternal - given the flag string, first checks the parent for the flag, then checks
 *                   this class' flags
 *
 *
 *********************************************************************************************/

bool Organism::setFlagInternal(const char *flagname, bool newval) {
   if (Physical::setFlagInternal(flagname, newval))
      return true;

	unsigned int i = locateInTable(flagname, oflag_list);
	if (i == UINT_MAX)
		return false;

   _orgflags[i] = true;
   return true;
}

/*********************************************************************************************
 * isFlagSetInternal - given the flag string, first checks the parent for the flag, then checks
 *                   this class' flags
 *
 *    Params:  flagname - flag to set
 *             results - if found, what the flag is set to
 *
 *    Returns: true if the flag was found, false otherwise
 *
 *********************************************************************************************/

bool Organism::isFlagSetInternal(const char *flagname, bool &results) {
   if (Physical::isFlagSetInternal(flagname, results))
      return true;

   std::string flagstr = flagname;
   lower(flagstr);

   size_t i=0;
   while ((oflag_list[i] != NULL) && (flagstr.compare(oflag_list[i]) != 0))
      i++;

   if (oflag_list[i] == NULL)
      return false;

   results = _orgflags[i];
	return true;
}

/*********************************************************************************************
 * getReview - returns the unprocessed review string of the indicated type.
 * getReviewProcessed - returns the review strings with the variables replaced by their values
 *
 *    Params:  review - an enum type indicating the review to return 
 *             reviewstr - the string version of the review--a bit slower as it has to do lookup
 *					buf - place to store the processed version of the review
 *					dir - direction to replace %d1 or %d2 with in the review
 *
 *    Returns: pointer to the review string
 *
 *********************************************************************************************/

const char *Organism::getReview(review_type review) {
	return _reviews[(unsigned int) review].c_str();
}

const char *Organism::getReview(const char *reviewstr) {
	std::string rstr = reviewstr;
	lower(rstr);
	unsigned int i=0;
	while (reviewlist[i] != NULL) {
		if (rstr.compare(reviewlist[i]) == 0)
			return getReview(review_type(i));
	}

	std::string msg("Unrecognized review type requested: ");
	msg += rstr;
	throw std::runtime_error(msg.c_str());
}

const char *Organism::getReviewProcessed(review_type review, std::string &buf, 
											Location::exitdirs dir, const char *customdir) {
	buf.clear();
	
	std::string name;

	if ((dir != Location::Custom) || (customdir != NULL)) {
		if (dir == Location::Custom) {
			if (customdir == NULL)
				throw std::invalid_argument("Custom exit defined but customdir parameter set to null.");

			_rformatter.changeMap('1', customdir);
			_rformatter.changeMap('2', customdir);
			_rformatter.changeMap('3', customdir);
		} else {
			_rformatter.changeMap('1', dir1_list[dir]);
			_rformatter.changeMap('2', dir2_list[dir]);
			_rformatter.changeMap('3', dir3_list[dir]);
		}
	}
	
	try {
		_rformatter.formatStr(_reviews[review].c_str(), buf);
	} catch (const format_error &e) {
		// On errors, set review to the error message
		buf = "Organism review error: ";
		buf += e.what();
	}
	
	return buf.c_str();
}

const char *Organism::getReviewProcessed(const char *reviewstr, std::string &buf,
                                 Location::exitdirs dir, const char *customdir) { 
   std::string rstr = reviewstr;
   lower(rstr);
   unsigned int i=0;
   while (reviewlist[i] != NULL) {
      if (rstr.compare(reviewlist[i]) == 0)
         return getReviewProcessed(review_type(i), buf, dir, customdir);
   }

   std::string msg("Unrecognized review type requested: ");
   msg += rstr;
   throw std::runtime_error(msg.c_str());
}


/*********************************************************************************************
 * setReview - assigns the passed in string to the appropriate review
 *
 *********************************************************************************************/

void Organism::setReview(review_type review, const char *new_review) {
	_reviews[review] = new_review;
}

/*********************************************************************************************
 * addBodyPart - when initializing a character, adds a body part to their self that can wear
 *               clothing or wield an item
 *
 *		Params:	name - the body part name, like hand
 *					group - the body part group, like arm
 *	
 *********************************************************************************************/

void Organism::addBodyPart(const char *group, const char *name) {
	_bodyparts.insert(std::pair<std::pair<std::string, std::string>, body_part>(
								std::pair<std::string, std::string>(group, name),
								body_part()));
}

/*********************************************************************************************
 * set/getBodyPartFlag - retrieves a body part from the list and sets or gets the bpflag
 *
 *    Params:  name - the body part name, like hand
 *             group - the body part group, like arm
 *					flag - body part flag to be setting or getting
 *					value - value to set the body part flag to
 *
 *		Throws: invalid_argument if the body part can't be found
 *	
 *********************************************************************************************/

void Organism::setBodyPartFlag(const char *group, const char *name, bpart_flags flag, bool value) {
	auto bpit = _bodyparts.find(std::pair<std::string, std::string>(group, name));
	if (bpit == _bodyparts.end()) {
		std::stringstream msg;
		msg << "Attempt to set flag on invalid body part '" << group << "," << name << ", Organism: " 
						 << getID();
		throw std::invalid_argument(msg.str().c_str());
	}
	bpit->second.bpflags[flag] = value;
}

bool Organism::getBodyPartFlag(const char *group, const char *name, bpart_flags flag) {
   auto bpit = _bodyparts.find(std::pair<std::string, std::string>(group, name));
   if (bpit == _bodyparts.end()) {
      std::stringstream msg;
      msg << "Attempt to get flag on invalid body part '" << group << "," << name << ", Organism: "
                   << getID();
      throw std::invalid_argument(msg.str().c_str());
   }
	return bpit->second.bpflags[flag];
}

/*********************************************************************************************
 * equip - wears or wields the entity passed in, assuming it's an equipment type 
 *
 *		Params:	equip_ptr - pointer to an entity object that should be an Equipment type
 *
 *		Returns: true for success, false for failure and errmsg is populated
 *
 *********************************************************************************************/
bool Organism::equip(std::shared_ptr<Physical> equip_ptr, std::string &errmsg) {
	std::shared_ptr<Equipment> eptr = std::dynamic_pointer_cast<Equipment>(equip_ptr);

	// Make sure it is an equipment type
	if (eptr == nullptr) {
		errmsg = "You can't equip that item.\n";
		return false;
	}

	// Loop through body parts to see if the necessary parts are available and it is not already worn
	for (unsigned int i=0; i<eptr->getEquipListSize(); i++) {
		int results = findBodyPartContained(eptr->getEquipListGroup(i), eptr->getEquipListName(i), eptr);
		if (results == -1) {
			std::stringstream errmsgstr;
			errmsgstr << "You are missing the required " << eptr->getEquipListName(i) << 
							" body part needed to equip the " << eptr->getTitle() << "\n";
			errmsg = errmsgstr.str();
			return false;
		} else if (results == 1) {
			errmsg = "You are already wearing that.\n";
			return false;
		}
	}

	// Now add the equipment
   for (unsigned int i=0; i<eptr->getEquipListSize(); i++) {
		if (!addBodyPartContained(eptr->getEquipListGroup(i), eptr->getEquipListName(i), eptr)) {
			errmsg = "Unexpected error.\n";
			return false;
		}
   }
	return true;
}

/*********************************************************************************************
 * remove - removes equipped entities
 *
 *    Params:  equip_ptr - pointer to an entity object that should be an Equipment type
 *
 *    Returns: true for success, false for failure and errmsg is populated
 *
 *********************************************************************************************/
bool Organism::remove(std::shared_ptr<Physical> equip_ptr, std::string &errmsg) {
   std::shared_ptr<Equipment> eptr = std::dynamic_pointer_cast<Equipment>(equip_ptr);

   // Make sure it is an equipment type
   if (eptr == nullptr) {
      errmsg = "You can't equip that item.\n";
      return false;
   }

   // Loop through body parts to see if the necessary parts are available and it is not already worn
   for (unsigned int i=0; i<eptr->getEquipListSize(); i++) {
      int results = remBodyPartContained(eptr->getEquipListGroup(i), eptr->getEquipListName(i), eptr);
      if (results <= 0) {
			errmsg = "You do not have that item equipped.\n";
			return false;
		}
   }

   return true;
}

/*********************************************************************************************
 * findBodyPartContained - checks if the given equipment is contained by the body part
 *
 *		Params:	name - the name of the bodypart - can be null to search all by a group
 *					
 *    Returns: 1 for contained, 0 for not, -1 for missing body part
 *
 *********************************************************************************************/

int Organism::findBodyPartContained(const char *name, const char *group, 
																		std::shared_ptr<Equipment> equip_ptr) const
{
	if ((name == NULL) && (group == NULL))
		throw std::runtime_error("findBodyPartContained both name and group were null. Only one is allowed.");

	// Simplest, fastest way is where both name and group are specified
	if ((name != NULL) && (group != NULL)) {
		std::pair<std::string, std::string> epair(name, group);

		auto bpptr = _bodyparts.find(epair);
		if (bpptr == _bodyparts.end())
			return -1;

		auto wornptr = bpptr->second.worn.begin();
		for ( ; wornptr != bpptr->second.worn.end(); wornptr++) {
			if (*wornptr == equip_ptr)
				return 1;
		}

	// another possibility is they want to search by group, so that'll be alphabetical in the map
	} else if (name == NULL) {
		auto m_it = _bodyparts.begin();
		while ((m_it != _bodyparts.end()) && (m_it->first.first.compare(group) == 0))
			m_it++;

		if (m_it == _bodyparts.end())
			return -1;

		while ((m_it != _bodyparts.end()) && (m_it->first.first.compare(group) == 0)) {
			auto wornptr = m_it->second.worn.begin();
			for ( ; wornptr != m_it->second.worn.end(); wornptr++) {
				if (*wornptr == equip_ptr)
					return 1;
			}
			m_it++;
		}
	// Slowest search, have to compare every single body part
	} else if (group == NULL) {
		auto m_it = _bodyparts.begin();
		while (m_it != _bodyparts.end()) {
			if (m_it->first.second.compare(name) == 0) {

				// search the bodyparts
				auto wornptr = m_it->second.worn.begin();
				for ( ; wornptr != m_it->second.worn.end(); wornptr++) {
					if (*wornptr == equip_ptr)
						return 1;
				}
			}
			m_it++;
		}
	}
   return 0;
}

/*********************************************************************************************
 * addBodyPartContained - adds the indicated equipment to the body part given it's name and group
 *
 *    Returns: true for success, false if the body part doesn't exist
 *
 *********************************************************************************************/

bool Organism::addBodyPartContained(const char *name, const char *group, std::shared_ptr<Equipment> equip_ptr) {
	std::pair<std::string, std::string> epair(name, group);

	auto bpptr = _bodyparts.find(epair);
	if (bpptr == _bodyparts.end())
		return false;

	bpptr->second.worn.push_back(equip_ptr);
	return true;
}

/*********************************************************************************************
 * remBodyPartContained - removes the indicated equipment to the body part given it's name and group
 *
 *    Returns: -1 for body part doesn't exist, 0 for not found, 1 for success
 *
 *********************************************************************************************/

int Organism::remBodyPartContained(const char *name, const char *group, std::shared_ptr<Equipment> equip_ptr) {
   std::pair<std::string, std::string> epair(name, group);

   auto bpptr = _bodyparts.find(epair);
   if (bpptr == _bodyparts.end())
      return -1;

   auto wornptr = bpptr->second.worn.begin();
   for ( ; wornptr != bpptr->second.worn.end(); bpptr++) {
      if (*wornptr == equip_ptr)
         bpptr->second.worn.erase(wornptr);
			return 1;
   }

   return 0;
}

/*********************************************************************************************
 * listContents - preps a string with a list of visible items, usually just the worn and
 *						wielded objects
 *
 *		Params:	buf - the list info is stored here
 *					exclude - a pointer to an entity that might be excluded from the list (NULL if none)
 *
 *********************************************************************************************/

const char *Organism::listContents(std::string &buf, const Physical *exclude) const {
   auto cit = _contained.begin();

		
   // Show worn equipment that is marked as visible
   for ( ; cit != _contained.end(); cit++) {
      std::shared_ptr<Equipment> eptr = std::dynamic_pointer_cast<Equipment>(*cit);

      if ((eptr == nullptr) || (*eptr == exclude))
         continue;

      buf += eptr->getRoomDesc();
      buf += "\n";
   }

   return buf.c_str();
}

/*********************************************************************************************
 * addTrait - assigns a trait to this organism via the parameter
 *
 *********************************************************************************************/

void Organism::addTrait(std::shared_ptr<Trait> new_trait) {
	_traits.push_back(new_trait);
}

/*********************************************************************************************
 * hasTrait - returns true if the indicated trait is found in the organism
 *
 *********************************************************************************************/
bool Organism::hasTrait(const char *trait_id) {
	std::string find_id(trait_id);

	for (unsigned int i=0; i<_traits.size(); i++) {
		if (find_id.compare(_traits[i]->getID()) == 0)
			return true;
	}
	return false;
}

/*********************************************************************************************
 * removeTrait - removes the indicated trait - returns false if not found, true otherwise
 *
 *********************************************************************************************/

bool Organism::removeTrait(const char *trait_id) {
   std::string find_id(trait_id);

   for (unsigned int i=0; i<_traits.size(); i++) {
      if (find_id.compare(_traits[i]->getID()) == 0)
			_traits.erase(_traits.begin() + i);
         return true;
   }
   return false;
}

/*********************************************************************************************
 * sendTraits - displays all the visible exits to the user for their location
 *
 *
 *********************************************************************************************/

void Organism::sendTraits() {

	std::stringstream msg;
   for (unsigned int i=0; i<_traits.size(); i++) {
		std::string name, category;

		_traits[i]->getNameID(name);
		_traits[i]->getZoneID(category);

		name[0] = toupper(name[0]);
		category[0] = toupper(category[0]);

		msg.str("");
		msg << "&+y" << category << "&*: " << name << "\n";
		sendMsg(msg.str().c_str());
   }

}

/*********************************************************************************************
 * getGameName - fills the buffer with the primary name that the game refers to this entity. 
 *               
 *
 *********************************************************************************************/

const char *Organism::getGameName(std::string &buf) const {
	if ((getTitle() == NULL) || (strlen(getTitle()) == 0))
		return getNameID(buf);

	buf = getTitle();
	return buf.c_str();
}

/*********************************************************************************************
 * listWhereWorn - Populates the string with the tags for where items are worn
 *
 *
 *********************************************************************************************/

void Organism::listWhereWorn(std::shared_ptr<Physical> obj, std::string &buf) const {

	std::shared_ptr<Equipment> eptr = std::dynamic_pointer_cast<Equipment>(obj);

	if (eptr == nullptr)
		return;

	// Hardcode in hands
	if (findBodyPartContained("hand", NULL, eptr) == 1) {
		buf += " (wielded)";	
	}

	if (findBodyPartContained(NULL, "head", eptr) == 1) {
		buf += " (worn on head)";
	}

	if (findBodyPartContained(NULL, "torso", eptr) == 1) {
		buf += " (worn on torso)";
	}

   if ((findBodyPartContained(NULL, "leftleg", eptr) == 1) && (findBodyPartContained(NULL, "rightleg", eptr) == 1)) {
      buf += " (worn on legs)";
   }

	if ((findBodyPartContained("foot", "rightleg", eptr) == 1) && (findBodyPartContained("foot", "rightleg", eptr) == 1)) {
		buf += " (worn on feet)";
	}
}


/*********************************************************************************************
 * dropAll - Drop all carried objects in the room
 *
 *
 *********************************************************************************************/

void Organism::dropAll() {
	// Unwear all
	auto bpptr = _bodyparts.begin();
	for ( ; bpptr != _bodyparts.end(); bpptr++)
		bpptr->second.worn.clear();

	// drop all
	while (_contained.size() > 0) {
		_contained.front()->movePhysical(getCurLoc());			
	}	
}


/*********************************************************************************************
 * canSee - does a bunch of checks to see if this organism can see in the room
 *
 *********************************************************************************************/

bool Organism::canSee() {
	std::shared_ptr<Location> cur_loc = std::dynamic_pointer_cast<Location>(getCurLoc());

	if (cur_loc == nullptr)
		throw std::runtime_error("canSee Organism cur_loc is not a Location entity.");

	if (!cur_loc->isLocFlagSet(Location::Dark))
		return true;

	if (containsLit(0))
		return true;

	if (cur_loc->containsLit(0))
		return true;

	return false;
}

/*********************************************************************************************
 * damage - damages the organism by decrementing the health attribute. If it falls below zero,
 *				then kills them
 *
 *		Params:	amount - the amount to damage the player
 *
 *		Returns: true if they die, false otherwise
 *
 *********************************************************************************************/

bool Organism::damage(unsigned int amount) {
	if (!incrAttribute("health", - (int) amount)) {
		throw std::runtime_error("Health attribute not found in this organism.");
	}	

	if (getAttribInt("health") <= 0) {
		sendMsg("You have died!\n");
		kill();
		return true;
	}
	return false;
}
