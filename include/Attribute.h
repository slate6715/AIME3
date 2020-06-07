#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H

#include <../external/pugixml.hpp>

class Attribute {
public:

	virtual ~Attribute();

	enum attr_type { Int, Float, String };

	virtual int getInt() const;
	virtual float getFloat() const;
	virtual const char *getStr() const;

	virtual void fillXMLNode(pugi::xml_node &anode) const = 0;

	virtual explicit operator int() const;
	virtual explicit operator float() const;
	virtual explicit operator const char *() const;

	virtual Attribute &operator = (int intval);
   virtual Attribute &operator = (float floatval);
   virtual Attribute &operator = (const char *strval);
   virtual Attribute &operator = (std::string &strval);

protected:
   Attribute();
   Attribute(const Attribute &copy_from);

private:
		
};

// analyzes the string and generates an appropriate Attribute type based on what it sees
Attribute *genAttrFromStr(const char *str);



class IntAttribute : public Attribute {
public:
	IntAttribute();
	IntAttribute(int setval);
	IntAttribute(const char *setval);
	IntAttribute(const IntAttribute &copy_from);
	virtual ~IntAttribute();

   virtual int getInt() const;

   virtual void fillXMLNode(pugi::xml_node &anode) const;

   virtual explicit operator int() const;

	virtual IntAttribute &operator = (int intval);
   virtual IntAttribute &operator = (const char *strval);

private:
	int _val;	
};

class FloatAttribute : public Attribute {
public:
   FloatAttribute();
	FloatAttribute(float setval);
	FloatAttribute(const char *setval);
   FloatAttribute(const FloatAttribute &copy_from);
   virtual ~FloatAttribute();

   virtual float getFloat() const;

   virtual void fillXMLNode(pugi::xml_node &anode) const;

   virtual explicit operator float() const;

   virtual FloatAttribute &operator = (float floatval);
   virtual FloatAttribute &operator = (const char *strval);

private:
   float _val;
};

class StrAttribute : public Attribute {
public:
   StrAttribute();
	StrAttribute(const char *setval);
   StrAttribute(const StrAttribute &copy_from);
   virtual ~StrAttribute();

   virtual const char *getStr() const;

   virtual void fillXMLNode(pugi::xml_node &anode) const;

   virtual explicit operator const char *() const;

   virtual StrAttribute &operator = (const char *strval);
   virtual StrAttribute &operator = (std::string &strval);

private:
   std::string _val;
};


#endif //ifndef attribute
