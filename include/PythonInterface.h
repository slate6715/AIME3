#ifndef PYTHON_INTERFACE_H
#define PYTHON_INTERFACE_H

#include <memory>

class Physical;
class Organism;

class IPhysical {
public:
   IPhysical(std::shared_ptr<Physical> eptr);
   IPhysical(const IPhysical &copy_from);

   void sendMsg(const char *msg);
	void sendMsgLoc(const char *msg);

	void moveTo(IPhysical new_loc);

   std::string getCurLocID();
	std::string getTitle();

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
