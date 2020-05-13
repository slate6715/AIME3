#ifndef ORGANISM_H
#define ORGANISM_H

#include <vector>
#include <bitset>
#include "Entity.h"
#include "Location.h"

/***************************************************************************************
 * Organism - a living entity that can usually be killed and sometimes moves around. Used
 *				  for players and NPCs. This is a generic class and should not be enstantiated
 *				  directly
 *
 ***************************************************************************************/
class Organism : public Entity 
{
public:
	
   virtual ~Organism();

	enum review_type { Standing, Entering, Leaving };
	enum org_attrib { Strength, Damage };

	// Virtual functions that do nothing for NPCs
   virtual bool sendFile(const char *filename) { (void) filename; return true; };
   // Send a message to this entity or its contents - class-specific behavior
   virtual void sendMsg(const char *msg, std::shared_ptr<Entity> exclude=nullptr)                                                                                      { (void) msg; (void) exclude; };
   virtual void sendMsg(std::string &msg, std::shared_ptr<Entity> exclude=nullptr)                                                                                     { (void) msg; (void) exclude; };
	virtual void sendPrompt() {};
	virtual void sendCurLocation() {};
	virtual void sendExits() {};

	virtual const char *getDesc() const { return _desc.c_str(); };
	const char *getReview(review_type review);
   const char *getReview(const char *reviewstr);
	const char *getReviewProcessed(review_type review, std::string &buf,
                                 Location::exitdirs dir=Location::Custom, const char *customdir=NULL);
   const char *getReviewProcessed(const char *reviewstr, std::string &buf,
                                 Location::exitdirs dir=Location::Custom, const char *customdir=NULL);
	void setReview(review_type review, const char *new_review);

   void setAttribute(org_attrib attr, int val);
   void setAttribute(org_attrib attr, float val);
   void setAttribute(org_attrib attr, const char *val);

   int getAttributeInt(org_attrib attr);
   float getAttributeFloat(org_attrib attr);
   const char *getAttributeStr(org_attrib attr);

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

   virtual bool getAttribInternal(const char *attrib, int &value);
   virtual bool getAttribInternal(const char *attrib, float &value);
   virtual bool getAttribInternal(const char *attrib, std::string &value);

private:
	std::string _desc;

	std::vector<std::string> _reviews;

	std::vector<Attribute> _org_attrib;	
	std::bitset<32> _orgflags;
};


#endif
