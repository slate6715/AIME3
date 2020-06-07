#ifndef ENTITYDB_H
#define ENTITYDB_H

#include <map>
#include <memory>
#include "Entity.h"

class Trait;

/***************************************************************************************
 * EntityDB - class that stores and manages the game entities. Entities are stored based
 *				  on a key of zone@id which means that all zone entities will be grouped
 *				  together for purposes of iteration.
 *
 ***************************************************************************************/
class EntityDB 
{
public:
	EntityDB();
   EntityDB(const EntityDB &copy_from);
   virtual ~EntityDB();

	int loadEntities(libconfig::Config &mud_cfg);
	int loadTraits(libconfig::Config &mud_cfg);

	std::shared_ptr<Entity> getEntity(const char *id);

	std::shared_ptr<Trait> getTrait(const char *id);

	// Removes all references to this item from the database objects`
	size_t purgeEntity(std::shared_ptr<Entity> item);
 
private:
	std::map<std::string, std::shared_ptr<Entity>> _db;

	std::map<std::string, std::shared_ptr<Trait>> _traits;
};


#endif
