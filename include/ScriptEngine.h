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

	void setActor(std::shared_ptr<Physical> actor) { _actor = actor; };
   void setTarget1(std::shared_ptr<Physical> target) { _target1 = target; };
   void setTarget2(std::shared_ptr<Physical> target) { _target2 = target; };

	const char *getErrMsg() { return _errmsg.c_str(); };

private:

	IMUD _access;
	
	std::shared_ptr<Physical> _actor;
	std::shared_ptr<Physical> _target1;
	std::shared_ptr<Physical> _target2;

	std::string _script;

	std::string _errmsg;
};

#endif
