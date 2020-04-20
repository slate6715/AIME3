#include "EntityDB.h"


/*********************************************************************************************
 * EntityDB (constructor) - Creates an empty EntityDB object
 *
 *********************************************************************************************/
EntityDB::EntityDB():
					_db() 
{


}


EntityDB::EntityDB(const EntityDB &copy_from):
					_db(copy_from._db) 
{

}


EntityDB::~EntityDB() {

}


