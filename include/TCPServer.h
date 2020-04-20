#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <memory>
#include "FileDesc.h"
#include "TCPConn.h"
#include "LogMgr.h"

/********************************************************************************************
 * TCPServer - Basic functionality to manage a server socket and a list of connections. 
 *             Includes functionality to manage an AES encryption key loaded from file.
 *
 *             handleConnection is the primary maintenance function. Calls all the TCPConn
 *             handleConnection functions. 
 ********************************************************************************************/

class TCPServer 
{
public:
   TCPServer(LogMgr &log, unsigned int _verbosity = 1);
   virtual ~TCPServer();

   virtual void bindSvr(const char *ip_addr, unsigned short port);
   void listenSvr();

   void shutdown();

   TCPConn *handleSocket();

   unsigned long getIPAddr() { return _sockfd.getIPAddr(); };
   unsigned short getPort() { return _sockfd.getPort(); };

protected:


private:
   // Class to manage the server socket
   SocketFD _sockfd;

	LogMgr &_log;

	// Access control
	bool _use_accesslist = false;
	bool _whitelist = false;
	std::string _whitelist_file;
};


#endif
