#include <boost/python.hpp>
#include <iostream>
#include "ScriptEngine.h"
#include "Organism.h"

using namespace boost::python;

ScriptEngine::ScriptEngine() {
	Py_Initialize();

   _main_module = std::unique_ptr<object>(new object);
   _main_namespace = std::unique_ptr<object>(new object);

   (*_main_module) = import("__main__");

   (*_main_namespace) = (*_main_module).attr("__dict__");

   (*_main_namespace)["IMUD"] = class_<IMUD>("IMUD")
                                 .def("getPhysical", &IMUD::getPhysical)
											.def("getScript", &IMUD::getScript)
											.def("sendMsgAll", &IMUD::sendMsgAll)
											.def("sendMsgExc", &IMUD::sendMsgExc)
											.def("addScript", &IMUD::addScript);
   (*_main_namespace)["Physical"] = class_<IPhysical>("Physical", init<const IPhysical &>())
											.def("__eq__", &IPhysical::operator ==)
											.def("__ne__", &IPhysical::operator !=)
											.def("__iter__", range(&IPhysical::begin, &IPhysical::end))
                                 .def("sendMsg", &IPhysical::sendMsg)
                                 .def("sendMsgExc", &IPhysical::sendMsgExc)
                                 .def("damage", &IPhysical::damage)
                                 .def("moveTo", &IPhysical::moveTo)
                                 .def("destroy", &IPhysical::destroy)
                                 .def("showLocation", &IPhysical::showLocation)
                                 .def("getCurLocID", &IPhysical::getCurLocID)
                                 .def("getCurLoc", &IPhysical::getCurLoc)
                                 .def("getCurLoc2", &IPhysical::getCurLoc2)
                                 .def("getDoorState", &IPhysical::getDoorState)
                                 .def("setDoorState", &IPhysical::setDoorState)
                                 .def("addIntAttribute", &IPhysical::addIntAttribute)
                                 .def("addFloatAttribute", &IPhysical::addFloatAttribute)
                                 .def("addStrAttribute", &IPhysical::addStrAttribute)
											.def("getIntAttribute", &IPhysical::getIntAttribute)
											.def("getFloatAttribute", &IPhysical::getFloatAttribute)
											.def("getStrAttribute", &IPhysical::getStrAttribute)
                                 .def("hasAttribute", &IPhysical::hasAttribute)
                                 .def("getTitle", &IPhysical::getTitle)
                                 .def("getID", &IPhysical::getID)
											.def("setExit", &IPhysical::setExit)
											.def("clrExit", &IPhysical::clrExit)
											.def("isEquipped", &IPhysical::isEquipped)
											.def("isEquippedContained", &IPhysical::isEquippedContained);
											
	(*_main_namespace)["Contained"] = class_<IContained>("IContained", init<const IContained &>())
                                 .def("getIntAttribute", &IContained::getIntAttribute)
                                 .def("getFloatAttribute", &IContained::getFloatAttribute)
                                 .def("getStrAttribute", &IContained::getStrAttribute)
                                 .def("hasAttribute", &IContained::hasAttribute)
                                 .def("getTitle", &IContained::getTitle)
											.def("getID", &IContained::getID);

   (*_main_namespace)["Script"] = class_<IScript>("IScript", init<const IScript &>())
                                 .def("loadVariable", &IScript::loadVariable)
											.def("setInterval", &IScript::setInterval);

}

ScriptEngine::~ScriptEngine() {

}

/*********************************************************************************************
 * intialize - sets up all the interfaces to the mud classes
 *
 *
 *********************************************************************************************/
void ScriptEngine::initialize() {
   // initialize our functions

	_main_module = std::unique_ptr<object>(new object);
	_main_namespace = std::unique_ptr<object>(new object);

   (*_main_module) = import("__main__");

   (*_main_namespace) = (*_main_module).attr("__dict__");

   (*_main_namespace)["MUD"] = ptr(&_access);
}



/*********************************************************************************************
 * getEntity - Given an id, gets an IPhysical that points to the Entity
 *
 * Throws: script_error if the entity is not found
 *
 *********************************************************************************************/

int ScriptEngine::execute(const char *script) {
	std::string scriptstr(script);
	return execute(scriptstr);
}

 
int ScriptEngine::execute(std::string &script) {

	initialize();

	// Initialize some specific elements to this call 

	for (unsigned int i=0; i<_variables.size(); i++) {
		(*_main_namespace)[_variables[i].first.c_str()] = ptr(&(_variables[i].second));
	}

   for (unsigned int i=0; i<_variable_ints.size(); i++) {
      (*_main_namespace)[_variable_ints[i].first.c_str()] = _variable_ints[i].second;
   }

   for (unsigned int i=0; i<_variable_floats.size(); i++) {
      (*_main_namespace)[_variable_floats[i].first.c_str()] = _variable_floats[i].second;
   }

   for (unsigned int i=0; i<_variable_strs.size(); i++) {
      (*_main_namespace)[_variable_strs[i].first.c_str()] = _variable_strs[i].second;
   }

	// Execute the script and handle any exceptions
	try {
		object ignored = exec(script.c_str(), (*_main_namespace));
	} catch (error_already_set &e) {
		PyObject *type, *value, *traceback;
		PyErr_Fetch(&type, &value, &traceback);
		PyErr_NormalizeException(&type, &value, &traceback);

		handle<> hType(type);
		handle<> hValue(allow_null(value));
		handle<> hTraceback(allow_null(traceback)); 
	
		object oTraceback(import("traceback"));
		object formatted_list;
		if (!hTraceback) {
			object format_exception_only(oTraceback.attr("format_exception_only"));
			formatted_list = format_exception_only(hType,hValue);
		} else {
			object format_exception(oTraceback.attr("format_exception"));
			formatted_list = format_exception(hType,hValue,hTraceback);
		}

 		object formatted = str("").join(formatted_list);
		_errmsg = extract<std::string>(formatted);

		// An exit telling the command to stop executing. I'm sure there's a better way
		// to catch this exception. Unfortunately don't know it yet.
		if (_errmsg.find("SystemExit: 1") != std::string::npos) {
			clearVariables();
			return 1;
		}

		std::cout << "Err2: " << _errmsg << "\n";
		
		PyErr_Restore(type, value, traceback);
		clearVariables();
		return -1;
	}

	clearVariables();
	
	return 0;
}


/*********************************************************************************************
 * setVariable - sets up the variable string to physical entity mapping for execution
 *
 *
 *********************************************************************************************/

void ScriptEngine::setVariable(const char *varname, std::shared_ptr<Physical> variable) {
	_variables.push_back(std::pair<std::string, IPhysical>(varname, IPhysical(variable)));
}

void ScriptEngine::setVariableConst(const char *varname, int variable) {

	_variable_ints.push_back(std::pair<std::string, int>(varname, variable));
}

void ScriptEngine::setVariableConst(const char *varname, float variable) {

	_variable_floats.push_back(std::pair<std::string, float>(varname, variable));
}

void ScriptEngine::setVariableConst(const char *varname, const char *variable) {

	_variable_strs.push_back(std::pair<std::string, std::string>(varname, variable));
}

void ScriptEngine::clearVariables() {
	_variables.clear();
	_variable_ints.clear();
	_variable_floats.clear();
	_variable_strs.clear();
}
