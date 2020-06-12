#ifndef PYTHON_INTERFACE_H
#define PYTHON_INTERFACE_H

#include <memory>

class Entity;
class Organism;

class IEntity {
public:
   IEntity(std::shared_ptr<Entity> eptr);
   IEntity(const IEntity &copy_from);

   void sendMsg(const char *msg);
	void sendMsgLoc(const char *msg);

	void moveTo(IEntity new_loc);

   std::string getCurLocID();
	std::string getTitle();

private:
   std::shared_ptr<Entity> _eptr;
};


class IMUD {
public:
   IMUD();
   ~IMUD();

	IEntity getEntity(const char *id);

private:

};



#endif	// Ifndef
