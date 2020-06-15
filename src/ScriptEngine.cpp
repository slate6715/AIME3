#include <boost/python.hpp>
#include <iostream>
#include "ScriptEngine.h"
#include "Organism.h"

using namespace boost::python;

ScriptEngine::ScriptEngine() {

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
   Py_Initialize();

   _main_module = import("__main__");

   _main_namespace = _main_module.attr("__dict__");

   _main_namespace["IMUD"] = class_<IMUD>("IMUD")
                                 .def("getPhysical", &IMUD::getPhysical);
   _main_namespace["Physical"] = class_<IPhysical>("Physical", init<const IPhysical &>())
                                 .def("sendMsg", &IPhysical::sendMsg)
											.def("sendMsgLoc", &IPhysical::sendMsgLoc)
											.def("moveTo", &IPhysical::moveTo)
											.def("getCurLocID", &IPhysical::getCurLocID)
											.def("getCurLoc", &IPhysical::getCurLoc)
											.def("getCurLoc2", &IPhysical::getCurLoc2)
											.def("getDoorState", &IPhysical::getDoorState)
											.def("setDoorState", &IPhysical::setDoorState)
											.def("getTitle", &IPhysical::getTitle);

   _main_namespace["MUD"] = ptr(&_access);

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

	// Initialize some specific elements to this call 
	IPhysical actor(_actor);
	IPhysical target1(_target1);
	IPhysical target2(_target2);

	if (_actor != nullptr)
		_main_namespace["actor"] = ptr(&actor);
	if (_target1 != nullptr)
		_main_namespace["target1"] = ptr(&target1);
	if (_target2 != nullptr)
		_main_namespace["target2"] = ptr(&target2);

	// Execute the script and handle any exceptions
	try {
		object ignored = exec(script.c_str(), _main_namespace);
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
		if (_errmsg.find("SystemExit: 1") != std::string::npos)
			return 1;

		std::cout << "Err2: " << _errmsg << "\n";
		
		PyErr_Restore(type, value, traceback);
		return -1;
	}
	
	
	return 0;
}


