#ifndef ORGANISM_H
#define ORGANISM_H

#include <vector>
#include <list>
#include <bitset>
#include <map>
#include "Physical.h"
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
	enum org_attrib { Strength, Constitution, Dexterity, Intelligence, Wisdom, Charisma, Experience, Damage, Last };

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
   virtual const char *getTitle() const { return NULL; };

	const char *getReview(review_type review);
   const char *getReview(const char *reviewstr);
	const char *getReviewProcessed(review_type review, std::string &buf,
                                 Location::exitdirs dir=Location::Custom, const char *customdir=NULL);
   const char *getReviewProcessed(const char *reviewstr, std::string &buf,
                                 Location::exitdirs dir=Location::Custom, const char *customdir=NULL);
	void setReview(review_type review, const char *new_review);

	// Set and get dynamic attributes
   void setAttribute(org_attrib attr, int val);
   void setAttribute(org_attrib attr, float val);
   void setAttribute(org_attrib attr, const char *val);
	void setAttribute(org_attrib attr, Attribute &value);

   int getAttributeInt(org_attrib attr);
   float getAttributeFloat(org_attrib attr);
   const char *getAttributeStr(org_attrib attr);

	// Manage body parts for wearing and wielding equipment
	void addBodyPart(const char *group, const char *name);
	void setBodyPartFlag(const char *group, const char *name, bpart_flags flag, bool value);
	bool getBodyPartFlag(const char *group, const char *name, bpart_flags flag);

	
	// Manage traits
	void addTrait(std::shared_ptr<Trait> new_trait);
	bool hasTrait(const char *trait_id);
	bool removeTrait(const char *trait_id);

	// Equip a piece of equipment (should already be in the organism's inventory
	bool equip(std::shared_ptr<Physical> equip_ptr, std::string &errmsg);
   bool remove(std::shared_ptr<Physical> equip_ptr, std::string &errmsg);

	virtual const char *listContents(std::string &buf, const Physical *exclude = NULL) const;	
	
	void listWhereWorn(std::shared_ptr<Physical> obj, std::list<std::string> &bodyparts);

protected:
	Organism(const char *id);	// Must be called from the child constructor
	Organism(const Organism &copy_from);

   virtual void saveData(pugi::xml_node &entnode) const;
   virtual int loadData(pugi::xml_node &entnode);

   virtual bool setFlagInternal(const char *flagname, bool newval);
   virtual bool isFlagSetInternal(const char *flagname, bool &results);

   virtual bool setAttribInternal(const char *attrib, int value);
   virtual bool setAttribInternal(const char *attrib, float value);
   virtual bool setAttribInternal(const char *attrib, const char *value);
	virtual bool setAttribInternal(const char *attrib, Attribute &value);

   virtual bool getAttribInternal(const char *attrib, int &value);
   virtual bool getAttribInternal(const char *attrib, float &value);
   virtual bool getAttribInternal(const char *attrib, std::string &value);

	virtual void fillAttrXMLNode(pugi::xml_node &anode) const;

	Attribute::attr_type getAttribTypeInternal(const char *attrib) const;

	bool addBodyPartContained(const char *name, const char *group, std::shared_ptr<Equipment> equip_ptr);
   int remBodyPartContained(const char *name, const char *group, std::shared_ptr<Equipment> equip_ptr);
	int findBodyPartContained(const char *name, const char *group, std::shared_ptr<Equipment> equip_ptr);

private:
	std::string _examine;

	std::vector<std::string> _reviews;

	std::vector<std::unique_ptr<Attribute>> _org_attrib;	
	std::bitset<32> _orgflags;

	std::map<std::pair<std::string, std::string>, body_part> _bodyparts;

	std::vector<std::shared_ptr<Trait>> _traits;
};


#endif
