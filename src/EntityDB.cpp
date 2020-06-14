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
#include "Equipment.h"
#include "NPC.h"
#include "Door.h"
#include "Trait.h"


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
 * loadPhysicals - reads the zones directory and loads all files in that directory. Does not
 *						necessarily care what files an entity is in but what its ID is that defines a
 *						zone membership.
 *
 *		Params: mud_cfg - holds config information
 *
 *		Returns: number of entities read in
 *
 *********************************************************************************************/

int EntityDB::loadPhysicals(libconfig::Config &mud_cfg) {

	std::stringstream errmsg;
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
			// If a parsing error, get the line number
			unsigned int linenum = getLineNumber(filepath.c_str(), result.offset);		
			errmsg << "Unable to open/parse zone file '" << filepath << "', (line: " << linenum << ") error: " << result.description();
         mudlog->writeLog(errmsg.str().c_str());
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
			std::shared_ptr<Physical> newptr(new_ent);
			new_ent->setSelfPtr(newptr);
         _db.insert(std::pair<std::string, std::shared_ptr<Physical>>(new_ent->getID(), newptr));
			count++;
		}
      // Get all static (non-moveable) objects
      for (pugi::xml_node stat = zonefile.child("static"); stat; stat = stat.next_sibling("static")) {
			Static *new_ent = new Static("temp");
         if (!new_ent->loadEntity(stat)) {
            std::stringstream msg;
            msg << "Bad format for static '" << new_ent->getID() << "', file '" << files[i] << "'";
            mudlog->writeLog(msg.str().c_str());
            delete new_ent;
            continue;
         }
         std::shared_ptr<Physical> newptr(new_ent);
         new_ent->setSelfPtr(newptr);
         _db.insert(std::pair<std::string, std::shared_ptr<Physical>>(new_ent->getID(), newptr));
         count++;
 			 
		}
      // Get all getable objects
      for (pugi::xml_node get_x = zonefile.child("getable"); get_x; get_x = get_x.next_sibling("getable")) {
         Getable *new_ent = new Getable("temp");
         if (!new_ent->loadEntity(get_x)) {
            std::stringstream msg;
            msg << "Bad format for getable '" << new_ent->getID() << "', file '" << files[i] << "'";
            mudlog->writeLog(msg.str().c_str());
            delete new_ent;
            continue;
         }
         std::shared_ptr<Physical> newptr(new_ent);
         new_ent->setSelfPtr(newptr);
         _db.insert(std::pair<std::string, std::shared_ptr<Physical>>(new_ent->getID(), newptr));
         count++;

      }
      // Get all locations
      for (pugi::xml_node get_x = zonefile.child("door"); get_x; get_x = get_x.next_sibling("door")) {
         Door *new_ent = new Door("temp");
         if (!new_ent->loadEntity(get_x)) {
            std::stringstream msg;
            msg << "Bad format for door '" << new_ent->getID() << "', file '" << files[i] << "'";
            mudlog->writeLog(msg.str().c_str());
            delete new_ent;
            continue;
         }
         std::shared_ptr<Physical> newptr(new_ent);
         new_ent->setSelfPtr(newptr);
         _db.insert(std::pair<std::string, std::shared_ptr<Physical>>(new_ent->getID(),newptr));
         count++;

      }

      // Get all equipment 
      for (pugi::xml_node get_x = zonefile.child("equipment"); get_x; get_x = 
																				get_x.next_sibling("equipment")) {
         Equipment *new_ent = new Equipment("temp");
         if (!new_ent->loadEntity(get_x)) {
            std::stringstream msg;
            msg << "Bad format for equipment '" << new_ent->getID() << "', file '" << files[i] << "'";
            mudlog->writeLog(msg.str().c_str());
            delete new_ent;
            continue;
         }
         std::shared_ptr<Physical> newptr(new_ent);
         new_ent->setSelfPtr(newptr);
         _db.insert(std::pair<std::string, std::shared_ptr<Physical>>(new_ent->getID(),newptr));
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
         std::shared_ptr<Physical> newptr(new_ent);
         new_ent->setSelfPtr(newptr);
         _db.insert(std::pair<std::string, std::shared_ptr<Physical>>(new_ent->getID(), newptr));
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
 * loadTraits - reads the traits directory and loads all files in that directory.
 *
 *    Params: mud_cfg - holds config information
 *
 *    Returns: number of entities read in
 *
 *********************************************************************************************/

int EntityDB::loadTraits(libconfig::Config &mud_cfg) {

   int count = 0;

   // Load the traits from the Actions directory
   std::string traitsdir;
   mud_cfg.lookupValue("datadir.traitsdir", traitsdir);

   std::vector<std::string> files;
   boost::filesystem::path p(traitsdir);

   if (!boost::filesystem::exists(traitsdir)) {
      std::string msg("traitsdir defined in config file doesn't appear to exist at: ");
      msg += traitsdir;
      throw std::runtime_error(msg.c_str());
   }
   boost::filesystem::directory_iterator start(p), end;
   std::transform(start, end, std::back_inserter(files), path_leaf_string());

   pugi::xml_document traitsfile;
   for (unsigned int i=0; i<files.size(); i++) {
      std::string filepath(traitsdir);
      filepath += "/";
      filepath += files[i].c_str();

      pugi::xml_parse_result result = traitsfile.load_file(filepath.c_str());

      if (!result) {
         std::string msg("Unable to open/parse traits file: ");
         msg += filepath;
         msg += ", error: ";
         msg += result.description();
         mudlog->writeLog(msg.c_str());
         continue;
      }

      // Get all traits
      for (pugi::xml_node loc = traitsfile.child("trait"); loc; loc = loc.next_sibling("trait")) {
         Trait *new_ent = new Trait("temp");
         if (!new_ent->loadEntity(loc)) {
            std::stringstream msg;
            msg << "Bad format for trait '" << new_ent->getID() << "', file '" << files[i] << "'";
            mudlog->writeLog(msg.str().c_str());
            delete new_ent;
            continue;
         }
         _traits.insert(std::pair<std::string, std::shared_ptr<Trait>>(new_ent->getID(),
                                                   std::shared_ptr<Trait>(new_ent)));
         count++;
      }
	}
	return count;
}

/*********************************************************************************************
 * getPhysical - retrieves the entity with the given id
 *
 *		Returns: shared_ptr to the entity, or set to null if not found
 *
 *********************************************************************************************/

std::shared_ptr<Physical> EntityDB::getPhysical(const char *id) {
	auto eptr = _db.find(id);
	
	if (eptr == _db.end())
		return std::shared_ptr<Physical>(nullptr);
	return eptr->second;
}

/*********************************************************************************************
 * getTrait - retrieves the trait with the given id
 *
 *    Returns: shared_ptr to the trait, or set to null if not found
 *
 *********************************************************************************************/

std::shared_ptr<Trait> EntityDB::getTrait(const char *id) {
   auto eptr = _traits.find(id);

   if (eptr == _traits.end())
      return std::shared_ptr<Trait>(nullptr);
   return eptr->second;
}

/*********************************************************************************************
 * purgePhysical - Removes all references to the parameter from the Entities in the database so
 *					  it can be safely removed
 *
 *		Returns: number of references to this object cleared
 *
 *********************************************************************************************/

size_t EntityDB::purgePhysical(std::shared_ptr<Physical> item) {
	size_t count = 0;

	// Loop through all items, purging the entity
	auto ent_it = _db.begin();
	for ( ; ent_it != _db.end(); ent_it++) {
		count += ent_it->second->purgePhysical(item);
	}
	return count;
}

