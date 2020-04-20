#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdexcept>
#include <strings.h>
#include <vector>
#include <iostream>
#include <memory>
#include <sstream>
#include "TCPServer.h"
#include "ALMgr.h"

TCPServer::TCPServer(LogMgr &log):
								_log(log),
								_sockfd()
{
}


TCPServer::~TCPServer() {

}

/**********************************************************************************************
 * bindSvr - Creates a network socket and sets it nonblocking so we can loop through looking for
 *           data. Then binds it to the ip address and port
 *
 *    Throws: socket_error for recoverable errors, runtime_error for unrecoverable types
 **********************************************************************************************/

void TCPServer::bindSvr(const char *ip_addr, short unsigned int port) {

   // Set the socket to nonblocking
   _sockfd.setNonBlocking();

   _sockfd.setReusable();

   // bind to the given ip address and port
   _sockfd.bindFD(ip_addr, port);
 
}

/**********************************************************************************************
 * listenSvr - Starts the server socket listening for incoming connections
 *
 *    Throws: socket_error for recoverable errors, runtime_error for unrecoverable types
 **********************************************************************************************/

// Simple function that simply starts the server listening
void TCPServer::listenSvr() {
   _sockfd.listenFD(5);

   std::string ipaddr_str;
   std::stringstream msg;
   _sockfd.getIPAddrStr(ipaddr_str);
   msg << "Server listening on IP " << ipaddr_str << "' port '" << _sockfd.getPort() << "'";
   _server_log.writeLog(msg.str().c_str());
}


/**********************************************************************************************
 * handleSocket - Checks the socket for incoming connections and validates against the whitelist.
 *                Accepts valid connections and adds them to the connection list.
 *
 *    Returns: pointer to a new connection if one was found, otherwise NULL
 *
 *    Throws: socket_error for recoverable errors, runtime_error for unrecoverable types
 **********************************************************************************************/

TCPConn *TCPServer::handleSocket() {
  
   // The socket has data, means a new connection 
   if (_sockfd.hasData()) {

      // Try to accept the connection
      TCPConn *new_conn = new TCPConn(_log, _verbosity);
      if (!new_conn->accept(_sockfd)) {
         _log.strerrLog("Data received on listening socket but accept failed.");
         return NULL;
      }

      // Get their IP Address string to use in logging
      std::string ipaddr_str;
      new_conn->getIPAddrStr(ipaddr_str);

	
		// If we need to check this IP address for validity	
		if (_use_accesslist) {
			// Check the whitelist
			ALMgr al(_whitelist_file.c_str(), _whitelist);

			// TODO: Need to also add DNS lookup for the whitelist

			// Check the IP address to see if it's allowed
			if (!al.isAllowed(new_conn->getIPAddr()))
			{
				// Disconnect the user
				new_conn->disconnect();

				// Log their attempted connection
				std::string msg = "Connection by IP address '";
				msg += ipaddr_str;
				msg += "' not allowed based on access list config.";
				_server_log.writeLog(msg);

				return NULL;
			}
		}

      std::string msg = "Connection from IP address '";
      msg += ipaddr_str;
      msg += "'.";
      _server_log.writeLog(msg);

      // Send an authentication string in cleartext

      return new_conn;
   }
   return NULL;
}

/**********************************************************************************************
 * handleConnections - Loops through the list of clients, running their functions to handle the
 *                     clients input/output.
 *
 *    Returns: number of new connections accepted
 *
 *    Throws: socket_error for recoverable errors, runtime_error for unrecoverable types
 **********************************************************************************************/

/// Moving to UserDB
/*
void TCPServer::handleConnections() {
   // Loop through our connections, handling them
   std::list<std::unique_ptr<TCPConn>>::iterator tptr = _connlist.begin();
   while (tptr != _connlist.end())
   {
      // If the client is not connected, then either reconnect or drop 
      if ((!(*tptr)->isConnected()) || ((*tptr)->getStatus() == TCPConn::s_none)) {
         // Might be trying to connect
         if ((*tptr)->getStatus() == TCPConn::s_connecting) {

            // If our retry timer hasn't expired....skip
            if ((*tptr)->reconnect > time(NULL)) {
               tptr++;
               continue;
            }

            unsigned long ip_addr = (*tptr)->getIPAddr();
            unsigned short port = (*tptr)->getPort();
            
            // Try to connect and handle failure
            try {
               (*tptr)->connect(ip_addr, port);
            } catch (socket_error &e) {
               std::stringstream msg;
               msg << "Connect to SID " << (*tptr)->getNodeID() << 
                        " failed when trying to send data. Msg: " << e.what();
               if (_verbosity >= 2)
                  std::cout << msg.str() << "\n";
               _server_log.writeLog(msg.str().c_str());
               (*tptr)->disconnect();
               (*tptr)->reconnect = time(NULL) + reconnect_delay;
               tptr++;
               continue;
            }
         // Else we're in a different state and there's not data waiting to be read
         } else if (!(*tptr)->isInputDataReady()) {
         // Log it
            std::string msg = "Node ID '";
            msg += (*tptr)->getNodeID();
            msg += "' lost connection.";
            _server_log.writeLog(msg);

            // Remove them from the connect list
            tptr = _connlist.erase(tptr);
            std::cout << "Connection disconnected.\n";
            return;
         }
         tptr++;
         continue;
      }

      // Process any user inputs
      (*tptr)->handleConnection();

      // Increment our iterator
      tptr++;
   }

}


/**********************************************************************************************
 * shutdown - Cleanly closes the socket FD.
 *
 *    Throws: socket_error for recoverable errors, runtime_error for unrecoverable types
 **********************************************************************************************/

void TCPServer::shutdown() {
   _server_log.writeLog("Server shutting down.");

   _sockfd.closeFD();
}

