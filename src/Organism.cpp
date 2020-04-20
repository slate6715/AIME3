#include "Organism.h"


/*********************************************************************************************
 * Organism (constructor) - Called by a child class to initialize any Organism elements
 *
 *********************************************************************************************/
Organism::Organism(const char *id):
								Entity(id) 
{


}

// Called by child class
Organism::Organism(const Organism &copy_from):
								Entity(copy_from)
{

}


Organism::~Organism() {

}


