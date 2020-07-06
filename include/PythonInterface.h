#ifndef PYTHON_INTERFACE_H
#define PYTHON_INTERFACE_H

#include <memory>
#include <list>

class Physical;
class Organism;
class Script;
class IMUD;
class IPhysical;

void checkNull(std::shared_ptr<Physical> ptr);

/**********************************************************************************
 * IContained - Wraps a Physical object that is contained in  
 * 
 **********************************************************************************/
class IContained {
public:
	IContained(std::shared_ptr<Physical> eptr):_eptr(eptr) {};
	~IContained() {};

	bool hasAttribute(const char *attr);

   int getIntAttribute(const char *attr);
   float getFloatAttribute(const char *attr);
   std::string getStrAttribute(const char *attr);

	std::string getID();
	std::string getTitle();
	
	friend class IPhysical;
	
private:
	std::shared_ptr<Physical> _eptr;
};

class IPhysical {
public:
   IPhysical(std::shared_ptr<Physical> eptr);
   IPhysical(const IPhysical &copy_from);

	~IPhysical() {};

	bool operator == (const IPhysical &comp);
	bool operator != (const IPhysical &comp);

	// Send a message to this entity - for locations, exclude can limit people
   void sendMsg(const char *msg);
	void sendMsgExc(const char *msg, IPhysical &exclude);

	void moveTo(IPhysical &new_loc);
	void destroy();

	void showLocation();
	bool damage(int amount);

	std::string getDoorState();
	void setDoorState(const char *state);
	
   std::string getCurLocID();
	IPhysical getCurLoc();
	IPhysical getCurLoc2();

	std::string getTitle();
	std::string getID();

	void addContained(IPhysical &target);
	bool isContained(IPhysical &target);
	bool isContainedID(const char *id);

	bool addIntAttribute(const char *attr, int value);
	bool addFloatAttribute(const char *attr, float value);
	bool addStrAttribute(const char *attr, const char *value);

   int getIntAttribute(const char *attr);
   float getFloatAttribute(const char *attr);
   std::string getStrAttribute(const char *attr);

	bool hasAttribute(const char *attr);

	bool setExit(const char *exit, IPhysical &new_exit);
	bool clrExit(const char *exit);

	bool isEquipped(const char *name, const char *group, IPhysical &equip_ptr);
	bool isEquippedContained(const char *name, const char *group, IContained &equip_ptr);

	friend class IContained;
	friend class IScript;
	friend class IMUD;

	class iterator : public std::iterator<
                           std::input_iterator_tag,
                           IContained,
                           IContained &,
                           IContained *,
                           IContained >
	{
	public:
		iterator(std::list<std::shared_ptr<Physical>>::iterator ptr);
		iterator(const IPhysical::iterator &copy_from);
		iterator operator++();
		iterator operator++(int junk);
		IContained &operator*();
		IContained *operator->(); 
		bool operator == (const iterator &rhs);
		bool operator != (const iterator &rhs);

	private:
		std::list<std::shared_ptr<Physical>>::iterator _iptr;
		IContained _cptr;
	};

	iterator begin();
	iterator end();

private:
   std::shared_ptr<Physical> _eptr;
};


class IScript {
public:
   IScript(std::shared_ptr<Script> sptr);
   IScript(const IScript &copy_from);

   ~IScript() {};

	void loadVariable(const char *varname, IPhysical &variable);

	void setInterval(float new_interval);

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
	int sendMsgExc(const char *msg, IPhysical &exclude);

	int addScript(IScript &the_script);

private:

};


#endif	// Ifndef
