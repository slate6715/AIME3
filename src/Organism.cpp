#include <sstream>
#include <climits>
#include "Organism.h"
#include "global.h"
#include "misc.h"
#include "Location.h"
#include "Attribute.h"

const char *reviewlist[] = {"standing", "entering", "leaving", NULL};

const char *dir1_list[] = {"north", "south", "east", "west", "up", "down", "northeast", "northwest", 
									"southeast", "southwest", "custom", NULL};
const char *dir2_list[] = {"the north", "the south", "the east", "the west", "up", "down", "the northeast", 
								  "the northwest", "the southeast", "the southwest", "custom", NULL}; 
const char *dir3_list[] = {"the north", "the south", "the east", "the west", "above", "below", "the northeast",
                          "the northwest", "the southeast", "the southwest", "custom", NULL};
const char *org_attriblist[] = {"Strength", "Damage", NULL};

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

   // Add organism data (none yet)
   // pugi::xml_attribute idnode = entnode.append_attribute("id");
   // idnode.set_value(_id.c_str());

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
	_org_attrib[attr] = val;	
}

void Organism::setAttribute(org_attrib attr, float val) {
   _org_attrib[attr] = val;
}

void Organism::setAttribute(org_attrib attr, const char *val) {
   _org_attrib[attr] = val;
}

int Organism::getAttributeInt(org_attrib attr) {
	return _org_attrib[attr].getInt();
}

float Organism::getAttributeFloat(org_attrib attr) {
	return _org_attrib[attr].getFloat();
}

const char *Organism::getAttributeStr(org_attrib attr) {
	return _org_attrib[attr].getStr();
}

bool Organism::setAttribInternal(const char *attrib, int value) {
   if (Entity::setAttribInternal(attrib, value))
      return true;

   unsigned int i = locateInTable(attrib, org_attriblist);
   if (i == UINT_MAX)
      return false;
	
	_org_attrib[i] = value;
	return true;
}

bool Organism::setAttribInternal(const char *attrib, float value) {
   if (Entity::setAttribInternal(attrib, value))
      return true;

   unsigned int i = locateInTable(attrib, org_attriblist);
   if (i == UINT_MAX)
      return false;

   _org_attrib[i] = value;
   return true;
}

bool Organism::setAttribInternal(const char *attrib, const char *value) {
   if (Entity::setAttribInternal(attrib, value))
      return true;

   unsigned int i = locateInTable(attrib, org_attriblist);
   if (i == UINT_MAX)
      return false;

   _org_attrib[i] = value;
   return true;
}

bool Organism::getAttribInternal(const char *attrib, int &value) {
   if (Entity::getAttribInternal(attrib, value))
      return true;

   unsigned int i = locateInTable(attrib, org_attriblist);
   if (i == UINT_MAX)
      return false;

   value = _org_attrib[i].getInt();
   return true;

}

bool Organism::getAttribInternal(const char *attrib, float &value) {
   if (Entity::getAttribInternal(attrib, value))
      return true;

   unsigned int i = locateInTable(attrib, org_attriblist);
   if (i == UINT_MAX)
      return false;

   value = _org_attrib[i].getFloat();
   return true;
}

bool Organism::getAttribInternal(const char *attrib, std::string &value) {
   if (Entity::getAttribInternal(attrib, value))
      return true;

   unsigned int i = locateInTable(attrib, org_attriblist);
   if (i == UINT_MAX)
      return false;

   value = _org_attrib[i].getStr();
   return true;
}

/*********************************************************************************************
 * setReview - assigns the passed in string to the appropriate review
 *
 *********************************************************************************************/

void Organism::setReview(review_type review, const char *new_review) {
	_reviews[review] = new_review;
}

