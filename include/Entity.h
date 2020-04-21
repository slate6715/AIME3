#ifndef ENTITY_H
#define ENTITY_H

#include <string>

/***************************************************************************************
 * Entity - the most abstract class of interactable MUD objects. This is a generic class
 *				so you would never create an Entity object. Hence, constructors are protected
 *				and there are some generic methods. 
 *
 ***************************************************************************************/
class Entity 
{
public:
   virtual ~Entity();

	const char *getID() { return _id.c_str(); };

protected:
	Entity(const char *id);	// Must be called from the child constructor
	Entity(const Entity &copy_from);

private:
	Entity();	// Should not be called

	std::string _id;
};


#endif
