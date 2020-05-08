#ifndef TCPCONN_H
#define TCPCONN_H

#include "FileDesc.h"
#include "LogMgr.h"

const int max_attempts = 2;

// Methods and attributes to manage a network connection, including tracking the username
// and a buffer for user input. Status tracks what "phase" of login the user is currently in
class TCPConn 
{
public:
   TCPConn();
	TCPConn(const TCPConn &copy_from);

   ~TCPConn();

	enum conn_status { Active, LostLink, Closing, Closed };
	conn_status getConnStatus() const { return _status; };

   bool accept(SocketFD &server);

	ssize_t handleConnection(time_t timeout);

	// Adds a string to the output buffer for eventual transmission
	void addOutput(const char *msg);

	// Sends the string to the connection immediately
   int sendText(const char *msg);

   bool getUserInput(std::string &buf);

	void startDisconnect();
	void lostLink(time_t timeout);

   bool isConnected();

   unsigned long getIPAddr() { return _connfd.getIPAddr(); };
   void getIPAddrStr(std::string &buf);

private:

   SocketFD _connfd;
 
   std::string _inputbuf;
	std::string _outputbuf;

	conn_status _status = Closed;

	time_t _lostlink_timeout = 0;
};


#endif
