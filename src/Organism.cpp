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
const char *org_attriblist[] = {"strength", "damage", NULL};

const char *oflag_list[] = {NULL};

/*********************************************************************************************
 * Organism (constructor) - Called by a child class to initialize any Organism elements
 *
 *********************************************************************************************/
Organism::Organism(const char *id):
								Entity(id) 
{
	// Review defaults
	_reviews.push_back("%n is standing here.");				// Standing
	_reviews.push_back("%n enters the room from %d3");	// Entering
	_reviews.push_back("%n departs the room %d1.");		// Leaving

	// Hardcoded body parts for now, but later on, could customize for race/class
	addBodyPart("head", "head");
	addBodyPart("head", "face");
	addBodyPart("head", "neck");
	addBodyPart("torso", "chest");
	addBodyPart("torso", "back");
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

	// Strength
	_org_attrib.push_back(std::unique_ptr<Attribute>(new IntAttribute()));

	// Damage
	_org_attrib.push_back(std::unique_ptr<Attribute>(new IntAttribute()));
}

// Called by child class
Organism::Organism(const Organism &copy_from):
								Entity(copy_from)
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
	Entity::saveData(entnode);

   pugi::xml_node xnode;
   pugi::xml_attribute xattr;

	// Save the desc
	xnode = entnode.append_child("desc");
	xnode.append_child(pugi::node_pcdata).set_value(_desc.c_str());

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
	if ((results = Entity::loadData(entnode)) != 1)
		return results;

	// Now populate organism data (none yet)
   pugi::xml_node node = entnode.child("desc");
   if (node == nullptr) {
      mudlog->writeLog("Organism save file missing mandatory 'desc' field.", 2);
      return 0;
   }
   _desc = node.child_value();

   for (pugi::xml_node review = entnode.child("reviewmsg"); review; review = 
																					review.next_sibling("reviewmsg")) {
      try {
         pugi::xml_attribute attr = review.attribute("type");
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
         pugi::xml_attribute attr = trait.attribute("id");
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

	return 1;
}

/*********************************************************************************************
 * setFlagInternal - given the flag string, first checks the parent for the flag, then checks
 *                   this class' flags
 *
 *
 *********************************************************************************************/

bool Organism::setFlagInternal(const char *flagname, bool newval) {
   if (Entity::setFlagInternal(flagname, newval))
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
   if (Entity::isFlagSetInternal(flagname, results))
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
	std::stringstream errmsg;
	buf.clear();
	
	size_t pos;
	size_t start = 0;
	std::string name;

	// Find the next %
	while ((pos = _reviews[review].find("%", start)) != std::string::npos) {
		// Add all before the % to the buffer		
		buf += _reviews[review].substr(start, pos-start);
		pos++;
		if (pos >= _reviews[review].size()) {
			errmsg << "Review '" << reviewlist[review] << "' for Organism '" << getID() << 
																	"' has percent with invalid format type following it";
			mudlog->writeLog(errmsg.str().c_str());
			return NULL;
		}

		switch(_reviews[review][pos]) {
		case 'd':
			pos++;
			if (pos >= _reviews[review].size()) {
				errmsg << "Review '" << reviewlist[review] << "' for Organism '" << getID() <<
                                                   "' has percent with invalid format type following it";
				mudlog->writeLog(errmsg.str().c_str());
				return NULL;
			}
			if (dir==Location::Custom) {
				buf += customdir;
			} else if (_reviews[review][pos] == '1') {
				buf += dir1_list[dir];
         } else if (_reviews[review][pos] == '2') {
            buf += dir2_list[dir];
         } else if (_reviews[review][pos] == '3') {
            buf += dir3_list[dir];
         } else {
            errmsg << "Review '" << reviewlist[review] << "' for Organism '" << getID() <<
                                                   "' has percent with invalid format type following it";
            mudlog->writeLog(errmsg.str().c_str());
            return NULL;
			}
			break;	
		case 'n':
			getNameID(name);
			name[0] = toupper(name[0]);
			buf += name;
			break;
		default:
         errmsg << "Review '" << reviewlist[review] << "' for Organism '" << getID() <<
                                                   "' has percent with invalid format type following it";
         mudlog->writeLog(errmsg.str().c_str());
         return NULL;
		}
		start = pos + 1;
	}
	if (start < _reviews[review].size()) {
		buf += _reviews[review].substr(start, _reviews[review].size()-start);
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
 * set/getAttribute - given the attribute, assigns the value. Different versionf rod ifferent
 *					variable types. 
 *
 *
 *********************************************************************************************/

void Organism::setAttribute(org_attrib attr, int val) {
	*(_org_attrib[attr]) = val;	
}

void Organism::setAttribute(org_attrib attr, float val) {
   *(_org_attrib[attr]) = val;
}

void Organism::setAttribute(org_attrib attr, const char *val) {
   *(_org_attrib[attr]) = val;
}

int Organism::getAttributeInt(org_attrib attr) {
	return _org_attrib[attr]->getInt();
}

float Organism::getAttributeFloat(org_attrib attr) {
	return _org_attrib[attr]->getFloat();
}

const char *Organism::getAttributeStr(org_attrib attr) {
	return _org_attrib[attr]->getStr();
}

bool Organism::setAttribInternal(const char *attrib, int value) {
   if (Entity::setAttribInternal(attrib, value))
      return true;

   unsigned int i = locateInTable(attrib, org_attriblist);
   if (i == UINT_MAX)
      return false;
	
	(*_org_attrib[i]) = value;
	return true;
}

bool Organism::setAttribInternal(const char *attrib, float value) {
   if (Entity::setAttribInternal(attrib, value))
      return true;

   unsigned int i = locateInTable(attrib, org_attriblist);
   if (i == UINT_MAX)
      return false;

   *(_org_attrib[i]) = value;
   return true;
}

bool Organism::setAttribInternal(const char *attrib, const char *value) {
   if (Entity::setAttribInternal(attrib, value))
      return true;

   unsigned int i = locateInTable(attrib, org_attriblist);
   if (i == UINT_MAX)
      return false;

   *(_org_attrib[i]) = value;
   return true;
}

bool Organism::getAttribInternal(const char *attrib, int &value) {
   if (Entity::getAttribInternal(attrib, value))
      return true;

   unsigned int i = locateInTable(attrib, org_attriblist);
   if (i == UINT_MAX)
      return false;

   value = _org_attrib[i]->getInt();
   return true;

}

bool Organism::getAttribInternal(const char *attrib, float &value) {
   if (Entity::getAttribInternal(attrib, value))
      return true;

   unsigned int i = locateInTable(attrib, org_attriblist);
   if (i == UINT_MAX)
      return false;

   value = _org_attrib[i]->getFloat();
   return true;
}

bool Organism::getAttribInternal(const char *attrib, std::string &value) {
   if (Entity::getAttribInternal(attrib, value))
      return true;

   unsigned int i = locateInTable(attrib, org_attriblist);
   if (i == UINT_MAX)
      return false;

   value = _org_attrib[i]->getStr();
   return true;
}

/*********************************************************************************************
 * fillAttrXMLNode - populates the parameter XML node with data from this entity's attributes.
 *                   polymorphic
 *
 *    Returns: true if the same, false otherwise
 *
 *********************************************************************************************/

void Organism::fillAttrXMLNode(pugi::xml_node &anode) const {
	// First call the parent
	Entity::fillAttrXMLNode(anode);

	// Populate with all the attributes
	pugi::xml_node nextnode;
	pugi::xml_attribute nextattr;

	for (unsigned int i=0; i < (int) Last; i++) {
		nextnode = anode.append_child("attribute");
		nextattr = nextnode.append_attribute("name");
		nextattr.set_value(org_attriblist[i]);
		_org_attrib[i]->fillXMLNode(nextnode);
	}

	
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
 *		Returns: 1 for success
 *					0 for can't be equipped due to no available hands for weapons or no room for
 *						wearable objects
 *					-1 if the entity is not an equipment type
 *					-2 if the body part doesn't exist
 *
 *********************************************************************************************/
int Organism::equip(std::shared_ptr<Entity> equip_ptr) {
	std::shared_ptr<Equipment> eptr = std::dynamic_pointer_cast<Equipment>(equip_ptr);

	if (eptr == nullptr)
		return -1;

	// First, verify there's room for this piece of equipment (todo)
	for (unsigned int i=0; i<eptr->getEquipListSize(); i++) {
		
	}

	// Now add the equipment
   for (unsigned int i=0; i<eptr->getEquipListSize(); i++) {
		if (!addBodyPartContained(eptr->getEquipListGroup(i), eptr->getEquipListName(i), eptr))
			return -2;
   }
	return 1;
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
 * listContents - preps a string with a list of visible items, usually just the worn and
 *						wielded objects
 *
 *		Params:	buf - the list info is stored here
 *					exclude - a pointer to an entity that might be excluded from the list (NULL if none)
 *
 *********************************************************************************************/

const char *Organism::listContents(std::string &buf, const Entity *exclude) const {
   auto cit = _contained.begin();

		
   // Show worn equipment that is marked as visible
   for ( ; cit != _contained.end(); cit++) {
      std::shared_ptr<Getable> gptr = std::dynamic_pointer_cast<Equipment>(*cit);

      if ((gptr == nullptr) || (*gptr == exclude))
         continue;

      buf += gptr->getRoomDesc();
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

