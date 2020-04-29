#include <libconfig.h++>
#include <vector>
#include <boost/filesystem.hpp>
#include <sstream>
#include <iostream>
#include "EntityDB.h"
#include "misc.h"
#include "global.h"
#include "Location.h"


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

/*********************************************************************************************
 * loadEntities - reads the zones directory and loads all files in that directory. Does not
 *						necessarily care what files an entity is in but what its ID is that defines a
 *						zone membership.
 *
 *		Params: mud_cfg - holds config information
 *
 *		Returns: number of entities read in
 *
 *********************************************************************************************/

int EntityDB::loadEntities(libconfig::Config &mud_cfg) {

	int count = 0;

   // Load the zones from the Actions directory
   std::string zonedir;
   mud_cfg.lookupValue("datadir.zonedir", zonedir);

   std::vector<std::string> files;
   boost::filesystem::path p(zonedir);

   if (!boost::filesystem::exists(zonedir)) {
      std::string msg("Zonedir defined in config file doesn't appear to exist at: ");
      msg += zonedir;
      throw std::runtime_error(msg.c_str());
   }
   boost::filesystem::directory_iterator start(p), end;
   std::transform(start, end, std::back_inserter(files), path_leaf_string());

   pugi::xml_document zonefile;
   for (unsigned int i=0; i<files.size(); i++) {
      std::cout << "Zone file: " << files[i] << std::endl;
      std::string filepath(zonedir);
      filepath += "/";
      filepath += files[i].c_str();

      pugi::xml_parse_result result = zonefile.load_file(filepath.c_str());

      if (!result) {
         std::string msg("Unable to open/parse zone file: ");
         msg += filepath;
			msg += ", error: ";
			msg += result.description();
         mudlog->writeLog(msg.c_str());
         continue;
      }

		// Get all locations
		for (pugi::xml_node loc = zonefile.child("Location"); loc; loc = loc.next_sibling("Location")) {
			Location *new_ent = new Location("temp");
			if (!new_ent->loadEntity(loc)) {
				std::stringstream msg;
				msg << "Bad location format for loc '" << new_ent->getID() << "', file '" << files[i] << "'";
				mudlog->writeLog(msg.str().c_str());
				delete new_ent;
				continue;
			}
         _db.insert(std::pair<std::string, std::shared_ptr<Entity>>(new_ent->getID(),
                                                   std::shared_ptr<Entity>(new_ent)));

         count++;
 			 
		}
	}
	return count;
 
}

/*********************************************************************************************
 * getEntity - retrieves the entity with the given id
 *
 *		Returns: shared_ptr to the entity, or set to null if not found
 *
 *********************************************************************************************/

std::shared_ptr<Entity> EntityDB::getEntity(const char *id) {
	auto eptr = _db.find(id);
	
	if (eptr == _db.end())
		return std::shared_ptr<Entity>(nullptr);
	return eptr->second;
}


