#ifndef PYTHON_INTERFACE_H
#define PYTHON_INTERFACE_H

#include <memory>

class Physical;
class Organism;

class IPhysical {
public:
   IPhysical(std::shared_ptr<Physical> eptr);
   IPhysical(const IPhysical &copy_from);

	// Send a message to this entity - for locations, exclude can limit people
   void sendMsg(const char *msg);
	void sendMsgLoc(const char *msg, IPhysical exclude);

	void moveTo(IPhysical new_loc);

	std::string getDoorState();
	void setDoorState(const char *state);
	
   std::string getCurLocID();
	IPhysical getCurLoc();
	IPhysical getCurLoc2();

	std::string getTitle();

	void addContained(IPhysical target);
	bool isContained(IPhysical target);
	bool isContainedID(const char *id);

private:
   std::shared_ptr<Physical> _eptr;
};


class IMUD {
public:
   IMUD();
   ~IMUD();

	IPhysical getPhysical(const char *id);

private:

};



#endif	// Ifndef
