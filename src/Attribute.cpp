#include <stdexcept>
#include <climits>
#include "Attribute.h"

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

// Special version, converts a string to an integer
IntAttribute &IntAttribute::operator = (const char *strval) {
	long val = strtol(strval, NULL, 10);
	if ((val > INT_MAX) || (val < INT_MIN)) {
		throw std::invalid_argument(
				"IntAttribute assigned string that converts to a number out of bounds for a signed integer");
	}
	return *this;
}

FloatAttribute::FloatAttribute():
                  Attribute(),
                  _val(0.0)
{

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

// Special version, converts a string to an integer
FloatAttribute &FloatAttribute::operator = (const char *strval) {
   _val = strtof(strval, NULL);
	return *this;
}

StrAttribute::StrAttribute():
                  Attribute(),
                  _val(0)
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

