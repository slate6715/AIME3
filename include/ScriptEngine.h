#ifndef SCRIPTENGINE_H
#define SCRIPTENGINE_H

#include <string>
#include <memory>
#include <boost/python.hpp>
#include <vector>

#include "PythonInterface.h"

class Organism;

class ScriptEngine {
public:
	ScriptEngine();
	~ScriptEngine();

	void initialize();

	int execute(const char *script);
	int execute(std::string &script);

	void setVariable(const char *varname, std::shared_ptr<Physical> variable);
	void setVariableConst(const char *varname, int variable);
	void setVariableConst(const char *varname, float variable);
	void setVariableConst(const char *varname, const char *variable);

	void clearVariables();	

	const char *getErrMsg() { return _errmsg.c_str(); };

private:

	IMUD _access;

	std::vector<std::pair<std::string, IPhysical>> _variables;
	std::vector<std::pair<std::string, int>> _variable_ints;
	std::vector<std::pair<std::string, float>> _variable_floats;
	std::vector<std::pair<std::string, std::string>> _variable_strs;
	
	std::string _script;

	std::string _errmsg;

	std::unique_ptr<boost::python::object> _main_module;
	std::unique_ptr<boost::python::object> _main_namespace;
};

#endif
