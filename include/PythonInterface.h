#ifndef PYTHON_INTERFACE_H
#define PYTHON_INTERFACE_H

#include <memory>

class Physical;
class Organism;
class Script;
class IMUD;

class IPhysical {
public:
   IPhysical(std::shared_ptr<Physical> eptr);
   IPhysical(const IPhysical &copy_from);

	~IPhysical() {};

	bool operator == (const IPhysical &comp);
	bool operator != (const IPhysical &comp);

	// Send a message to this entity - for locations, exclude can limit people
   void sendMsg(const char *msg);
	void sendMsgExc(const char *msg, IPhysical exclude);

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

	bool addIntAttribute(const char *attr, int value);
	bool addFloatAttribute(const char *attr, float value);
	bool addStrAttribute(const char *attr, const char *value);

	bool hasAttribute(const char *attr);

	bool setExit(const char *exit, IPhysical new_exit);
	bool clrExit(const char *exit);

	friend class IScript;
	friend class IMUD;

private:
   std::shared_ptr<Physical> _eptr;
};

class IScript {
public:
   IScript(std::shared_ptr<Script> sptr);
   IScript(const IScript &copy_from);

   ~IScript() {};

	void loadVariable(const char *varname, IPhysical variable);

	friend class IMUD;

private:
   std::shared_ptr<Script> _sptr;
};


class IMUD {
public:
   IMUD();
   ~IMUD();

	IPhysical getPhysical(const char *id);
	IScript getScript(const char *id);

	int sendMsgAll(const char *msg);
	int sendMsgExc(const char *msg, IPhysical exclude);

	int addScript(IScript the_script);

private:

};


#endif	// Ifndef
