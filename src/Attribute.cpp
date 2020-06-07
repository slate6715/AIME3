#include <stdexcept>
#include <climits>
#include <regex>
#include "Attribute.h"

// analyzes the string and generates an appropriate Attribute type based on what it sees
Attribute *genAttrFromStr(const char *str) {
	std::string val(str);
	std::regex intcheck("[-+]?[0-9]+");				// Int has only numbers (optional sign)
	std::regex floatcheck("[-+]?[0-9]*\\.[0-9]+"); // Float has a period followed by at least one number

	if (std::regex_match(val, intcheck))
		return new IntAttribute(str);
	else if (std::regex_match(val, floatcheck))
		return new FloatAttribute(str);
	else
		return new StrAttribute(str);
}

Attribute::Attribute() {

}

Attribute::Attribute(const Attribute &copy_from) {
	(void) copy_from;
}

Attribute::~Attribute() {

}

// **** Parent classes for when child classes are not defined, raise an error
int Attribute::getInt() const {
	throw std::invalid_argument("getInt - attribute type not an IntAttribute");
}

float Attribute::getFloat() const {
   throw std::invalid_argument("getFloat - attribute type not a FloatAttribute");
}

const char *Attribute::getStr() const {
   throw std::invalid_argument("getStr - attribute type not an StrAttribute");
}


Attribute::operator int() const {
   throw std::invalid_argument("operator & - attribute type not an IntAttribute");
}

Attribute::operator float() const {
   throw std::invalid_argument("operator & - attribute type not a FloatAttribute");
}

Attribute::operator const char *() const {
   throw std::invalid_argument("operator & - attribute type not a StrAttribute");

}

Attribute &Attribute::operator = (int intval) {
	(void) intval;
   throw std::invalid_argument("operator = - attribute type not an IntAttribute");
}

Attribute &Attribute::operator = (float floatval) {
	(void) floatval;
   throw std::invalid_argument("operator = - attribute type not a FloatAttribute");
}

Attribute &Attribute::operator = (const char *strval) {
	(void) strval;
   throw std::invalid_argument("operator = - attribute type not a StringAttribute");
}

Attribute &Attribute::operator = (std::string &strval) {
	(void) strval;
   throw std::invalid_argument("operator = - attribute type not a StringAttribute");
}

IntAttribute::IntAttribute():
						Attribute(),
						_val(0)
{

}

IntAttribute::IntAttribute(int setval):
						Attribute(),
						_val(setval)
{
}

/*********************************************************************************************
 * IntAttribute (constructor) - special version of the constructor that converts a string to an integer
 *
 *    Throws: std::invalid_argument or std::out_of_range on a bad integer
 *
 *********************************************************************************************/

IntAttribute::IntAttribute(const char *setval):
                  Attribute()
{
	operator = (setval);
}

IntAttribute::IntAttribute(const IntAttribute &copy_from):
											Attribute(copy_from)
{
	
}

IntAttribute::~IntAttribute() {

}

int IntAttribute::getInt() const {
	return _val;
}

IntAttribute::operator int() const {
	return _val;
}

IntAttribute &IntAttribute::operator = (int intval) {
	_val = intval;
	return *this;
}

/*********************************************************************************************
 * fillXMLNode = populates an XML node with type and value information
 *
 *
 *********************************************************************************************/

void IntAttribute::fillXMLNode(pugi::xml_node &anode) const {
   pugi::xml_attribute aattr = anode.append_attribute("type");
   aattr.set_value("int");

   aattr = anode.append_attribute("value");
   aattr.set_value(_val);

}

/*********************************************************************************************
 * operator = - special version of the overloaded operator that converts a string to an integer
 *
 *		Throws: std::invalid_argument or std::out_of_range on a bad integer
 *
 *********************************************************************************************/

IntAttribute &IntAttribute::operator = (const char *strval) {
	_val = std::stoi(strval);
	return *this;
}

FloatAttribute::FloatAttribute():
                  Attribute(),
                  _val(0.0)
{

}

FloatAttribute::FloatAttribute(float setval):
                  Attribute(),
                  _val(setval)
{
}

/*********************************************************************************************
 * IntAttribute (constructor) - special version of the constructor that converts a string to a float
 *
 *    Throws: std::invalid_argument or std::out_of_range on a bad integer
 *
 *********************************************************************************************/

FloatAttribute::FloatAttribute(const char *setval):
                  Attribute()
{
   operator = (setval);
}

FloatAttribute::FloatAttribute(const FloatAttribute &copy_from):
													Attribute(copy_from)
{

}

FloatAttribute::~FloatAttribute() {

}

float FloatAttribute::getFloat() const {
   return _val;
}

FloatAttribute::operator float() const {
   return _val;
}

FloatAttribute &FloatAttribute::operator = (float floatval) {
   _val = floatval;
	return *this;
}

/*********************************************************************************************
 * fillXMLNode = populates an XML node with type and value information
 *
 *
 *********************************************************************************************/

void FloatAttribute::fillXMLNode(pugi::xml_node &anode) const {
   pugi::xml_attribute aattr = anode.append_attribute("type");
   aattr.set_value("float");

   aattr = anode.append_attribute("value");
   aattr.set_value(_val);

}


/*********************************************************************************************
 * operator = - special version of the overloaded operator that converts a string to a float
 *
 *    Throws: std::invalid_argument or std::out_of_range on a bad integer
 *
 *********************************************************************************************/

FloatAttribute &FloatAttribute::operator = (const char *strval) {
   _val = std::stof(strval);
   return *this;
}


StrAttribute::StrAttribute():
                  Attribute(),
                  _val(0)
{

}

StrAttribute::StrAttribute(const char *setval):
                  Attribute(),
                  _val(setval)
{
}

StrAttribute::StrAttribute(const StrAttribute &copy_from):
                                 Attribute(copy_from)
{

}

StrAttribute::~StrAttribute() {

}

const char *StrAttribute::getStr() const {
   return _val.c_str();
}

StrAttribute::operator const char *() const {
   return _val.c_str();
}

StrAttribute &StrAttribute::operator = (const char *strval) {
   _val = strval;
	return *this;
}

StrAttribute &StrAttribute::operator = (std::string &strval) {
   _val = strval;
	return *this;
}

/*********************************************************************************************
 * fillXMLNode = populates an XML node with type and value information
 *
 *
 *********************************************************************************************/

void StrAttribute::fillXMLNode(pugi::xml_node &anode) const {
   pugi::xml_attribute aattr = anode.append_attribute("type");
   aattr.set_value("str");

   aattr = anode.append_attribute("value");
   aattr.set_value(_val.c_str());

}


