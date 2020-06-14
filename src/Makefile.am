bindir = ../bin
bin_PROGRAMS = aime3

aime3_SOURCES = Action.cpp ActionMgr.cpp actions.cpp ALMgr.cpp Attribute.cpp Door.cpp Entity.cpp EntityDB.cpp Equipment.cpp FileDesc.cpp GameHandler.cpp Getable.cpp Handler.cpp Location.cpp LogMgr.cpp LoginHandler.cpp main.cpp misc.cpp MUD.cpp NPC.cpp Organism.cpp PageHandler.cpp Physical.cpp Player.cpp PythonInterface.cpp ../external/pugixml.cpp ScriptEngine.cpp Static.cpp TCPConn.cpp TCPServer.cpp Trait.cpp UserMgr.cpp 
aime3_CPPFLAGS = -Wall -Wextra -Wsign-conversion ${PYTHON_CPPFLAGS}
aime3_LDFLAGS = -pthread ${PYTHON_EXTRA_LDFLAGS}
aime3_LDADD = -lconfig++ -lboost_filesystem -lboost_system -lboost_python3 ${PYTHON_LIBS} ${PYTHON_EXTRA_LIBS} ${PYTHON_EXTRA_LIBS} ${BOOST_PYTHON_LIB}

