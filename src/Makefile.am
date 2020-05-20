bindir = ../bin
bin_PROGRAMS = aime3

aime3_SOURCES = main.cpp MUD.cpp Entity.cpp EntityDB.cpp UserMgr.cpp Organism.cpp Player.cpp TCPServer.cpp FileDesc.cpp LogMgr.cpp TCPConn.cpp Handler.cpp LoginHandler.cpp GameHandler.cpp ALMgr.cpp misc.cpp ../external/pugixml.cpp Action.cpp ActionMgr.cpp PageHandler.cpp actions.cpp Location.cpp Static.cpp Getable.cpp Attribute.cpp NPC.cpp Equipment.cpp 
aime3_CPPFLAGS = -Wall -Wextra -Wsign-conversion ${PYTHON_CPPFLAGS}
aime3_LDFLAGS = -pthread ${PYTHON_EXTRA_LDFLAGS}
aime3_LDADD = -lconfig++ -lboost_filesystem -lboost_system ${PYTHON_LIBS} ${PYTHON_EXTRA_LIBS} ${PYTHON_EXTRA_LIBS} ${BOOST_PYTHON_LIB}

