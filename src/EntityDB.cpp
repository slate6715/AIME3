#include <libconfig.h++>
#include <vector>
#include <boost/filesystem.hpp>
#include <sstream>
#include <iostream>
#include "EntityDB.h"
#include "misc.h"
#include "global.h"
#include "Location.h"
#include "Static.h"
#include "Getable.h"
#include "NPC.h"


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
		for (pugi::xml_node loc = zonefile.child("location"); loc; loc = loc.next_sibling("location")) {
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
      // Get all locations
      for (pugi::xml_node stat = zonefile.child("static"); stat; stat = stat.next_sibling("static")) {
			Static *new_ent = new Static("temp");
         if (!new_ent->loadEntity(stat)) {
            std::stringstream msg;
            msg << "Bad format for static '" << new_ent->getID() << "', file '" << files[i] << "'";
            mudlog->writeLog(msg.str().c_str());
            delete new_ent;
            continue;
         }
         _db.insert(std::pair<std::string, std::shared_ptr<Entity>>(new_ent->getID(),
                                                   std::shared_ptr<Entity>(new_ent)));
         count++;
 			 
		}
      // Get all locations
      for (pugi::xml_node get_x = zonefile.child("getable"); get_x; get_x = get_x.next_sibling("getable")) {
         Getable *new_ent = new Getable("temp");
         if (!new_ent->loadEntity(get_x)) {
            std::stringstream msg;
            msg << "Bad format for getable '" << new_ent->getID() << "', file '" << files[i] << "'";
            mudlog->writeLog(msg.str().c_str());
            delete new_ent;
            continue;
         }
         _db.insert(std::pair<std::string, std::shared_ptr<Entity>>(new_ent->getID(),
                                                   std::shared_ptr<Entity>(new_ent)));
         count++;

      }
		// GEt all NPCs
      for (pugi::xml_node get_x = zonefile.child("npc"); get_x; get_x = get_x.next_sibling("npc")) {
         NPC *new_ent = new NPC("temp");
         if (!new_ent->loadEntity(get_x)) {
            std::stringstream msg;
            msg << "Bad format for NPC '" << new_ent->getID() << "', file '" << files[i] << "'";
            mudlog->writeLog(msg.str().c_str());
            delete new_ent;
            continue;
         }
         _db.insert(std::pair<std::string, std::shared_ptr<Entity>>(new_ent->getID(),
                                                   std::shared_ptr<Entity>(new_ent)));
         count++;

      }

	}


	// Now go through linking entities together
	auto ent_it = _db.begin();
	for (; ent_it != _db.end(); ent_it++) {
		ent_it->second->addLinks(*this, ent_it->second);
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

/*********************************************************************************************
 * purgeEntity - Removes all references to the parameter from the Entities in the database so
 *					  it can be safely removed
 *
 *		Returns: number of references to this object cleared
 *
 *********************************************************************************************/

size_t EntityDB::purgeEntity(std::shared_ptr<Entity> item) {
	size_t count = 0;

	// Loop through all items, purging the entity
	auto ent_it = _db.begin();
	for ( ; ent_it != _db.end(); ent_it++) {
		count += ent_it->second->purgeEntity(item);
	}
	return count;
}

