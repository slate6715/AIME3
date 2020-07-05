#ifndef ORGANISM_H
#define ORGANISM_H

#include <vector>
#include <list>
#include <bitset>
#include <map>
#include "Physical.h"
#include "StrFormatter.h"
#include "Location.h"

class Equipment;
class Trait;
class Getable;

enum bpart_flags { CanWield, Damaged, Missing };

struct body_part {
	std::bitset<32> bpflags;
	std::list<std::shared_ptr<Equipment>> worn;
};

/***************************************************************************************
 * Organism - a living entity that can usually be killed and sometimes moves around. Used
 *				  for players and NPCs. This is a generic class and should not be enstantiated
 *				  directly
 *
 ***************************************************************************************/
class Organism : public Physical 
{
public:
	
   virtual ~Organism();
	
	enum review_type { Standing, Entering, Leaving };

	// Gets the primary reference name the game refers to this organism by
   virtual const char *getGameName(std::string &buf) const;

	// Virtual functions that do nothing for NPCs
   virtual bool sendFile(const char *filename) { (void) filename; return true; };
   // Send a message to this entity or its contents - class-specific behavior
   virtual void sendMsg(const char *msg, std::shared_ptr<Physical> exclude=nullptr)                                                                                      { (void) msg; (void) exclude; };
   virtual void sendMsg(std::string &msg, std::shared_ptr<Physical> exclude=nullptr)                                                                                     { (void) msg; (void) exclude; };
 	virtual void sendPrompt() {};
	virtual void sendCurLocation() {};
	virtual void sendExits() {};
	virtual void sendTraits();

	// Series of functions that retrieve class attribute data
	virtual const char *getExamine() const { return _examine.c_str(); };

	const char *getReview(review_type review);
   const char *getReview(const char *reviewstr);
	const char *getReviewProcessed(review_type review, std::string &buf,
                                 Location::exitdirs dir=Location::Custom, const char *customdir=NULL);
   const char *getReviewProcessed(const char *reviewstr, std::string &buf,
                                 Location::exitdirs dir=Location::Custom, const char *customdir=NULL);
	void setReview(review_type review, const char *new_review);

	const char *getTitle() const { return _title.c_str(); };

	// Manage body parts for wearing and wielding equipment
	void addBodyPart(const char *group, const char *name);
	void setBodyPartFlag(const char *group, const char *name, bpart_flags flag, bool value);
	bool getBodyPartFlag(const char *group, const char *name, bpart_flags flag);

	int findBodyPartContained(const char *name, const char *group, std::shared_ptr<Equipment> equip_ptr) const;

	// Manage traits
	void addTrait(std::shared_ptr<Trait> new_trait);
	bool hasTrait(const char *trait_id);
	bool removeTrait(const char *trait_id);

	// Is the organism able to see in this room
	bool canSee();

	// Equip a piece of equipment (should already be in the organism's inventory
	bool equip(std::shared_ptr<Physical> equip_ptr, std::string &errmsg);
   bool remove(std::shared_ptr<Physical> equip_ptr, std::string &errmsg);

	std::shared_ptr<Physical> getEquipped(const char *group, const char *name);
	bool isEquipped(const char *group, const char *name);

	virtual void kill() = 0;
	virtual void dropAll();

	virtual const char *listContents(std::string &buf, const Physical *exclude = NULL) const;	
	
	void listWhereWorn(std::shared_ptr<Physical> obj, std::string &buf) const;

protected:
	Organism(const char *id);	// Must be called from the child constructor
	Organism(const Organism &copy_from);

   virtual void saveData(pugi::xml_node &entnode) const;
   virtual int loadData(pugi::xml_node &entnode);

   virtual bool setFlagInternal(const char *flagname, bool newval);
   virtual bool isFlagSetInternal(const char *flagname, bool &results);

	bool addBodyPartContained(const char *name, const char *group, std::shared_ptr<Equipment> equip_ptr);
   int remBodyPartContained(const char *name, const char *group, std::shared_ptr<Equipment> equip_ptr);

	StrFormatter _rformatter;

private:
	std::string _title;
	std::string _examine;

	std::vector<std::string> _reviews;

	std::bitset<32> _orgflags;

	std::map<std::pair<std::string, std::string>, body_part> _bodyparts;

	std::vector<std::shared_ptr<Trait>> _traits;
};


#endif
