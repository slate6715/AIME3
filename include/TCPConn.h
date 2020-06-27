#ifndef TCPCONN_H
#define TCPCONN_H

#include <mutex>
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

	void setPreWrite(const char *str);
	void setPostWrite(const char *str);

   bool accept(SocketFD &server);

	ssize_t handleConnection(time_t timeout);

	// Adds a string to the output buffer for eventual transmission
	void addOutput(const char *msg);

	// Checks for input or output
	bool hasInput() { return (_inputbuf.size() > 0); };
	bool hasOutput() { return (_outputbuf.size() > 0); };

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

	std::string _prewrite;
	std::string _postwrite;

	conn_status _status = Closed;

	time_t _lostlink_timeout = 0;

	std::mutex _conn_mutex;

};


#endif
