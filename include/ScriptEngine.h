#ifndef SCRIPTENGINE_H
#define SCRIPTENGINE_H

#include <string>
#include <memory>
#include <boost/python.hpp>

#include "PythonInterface.h"

class Organism;

class ScriptEngine {
public:
	ScriptEngine();
	ScriptEngine(const char *script);
	ScriptEngine(std::string &script);
	~ScriptEngine();

	void initialize(boost::python::object &main_namespace);

	int execute();

	void setActor(std::shared_ptr<Entity> actor) { _actor = actor; };
   void setTarget1(std::shared_ptr<Entity> target) { _target1 = target; };
   void setTarget2(std::shared_ptr<Entity> target) { _target2 = target; };

	const char *getErrMsg() { return _errmsg.c_str(); };

private:

	IMUD _access;
	
	std::shared_ptr<Entity> _actor;
	std::shared_ptr<Entity> _target1;
	std::shared_ptr<Entity> _target2;

	std::string _script;

	std::string _errmsg;
};

#endif
