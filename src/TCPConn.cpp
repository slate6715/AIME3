#include <stdexcept>
#include <strings.h>
#include <unistd.h>
#include <cstring>
#include <algorithm>
#include <iostream>
#include "TCPConn.h"
#include "strfuncts.h"

TCPConn::TCPConn():
					_connfd(),
					_inputbuf(""),
					_outputbuf(""),
					_is_connected(false)
{

}

TCPConn::TCPConn(const TCPConn &copy_from):
               _connfd(),
               _inputbuf(copy_from._inputbuf),
               _outputbuf(copy_from._outputbuf),
					_is_connected(false)
{
}

TCPConn::~TCPConn() {

}

/**********************************************************************************************
 * accept - simply calls the acceptFD FileDesc method to accept a connection on a server socket.
 *
 *    Params: server - an open/bound server file descriptor with an available connection
 *
 *    Throws: socket_error for recoverable errors, runtime_error for unrecoverable types
 **********************************************************************************************/

bool TCPConn::accept(SocketFD &server) {
   if (_connfd.acceptFD(server))
	{
		_is_connected = true;
		return true;
	}

	return false;
}

/**********************************************************************************************
 * sendText - simply calls the sendText FileDesc method to send a string to this FD
 *
 *    Params:  msg - the string to be sent
 *             size - if we know how much data we should expect to send, this should be populated
 *
 *    Throws: runtime_error for unrecoverable errors
 **********************************************************************************************/

int TCPConn::sendText(const char *msg) {
   if (_connfd.writeFD(msg, strlen(msg)) < 0) {
      return -1;  
   }
   return 0;
}

/**********************************************************************************************
 * addOutput - adds this string to the output string buffer for eventual transmission
 *
 *    Params:  msg - the string to be sent
 *
 **********************************************************************************************/

void TCPConn::addOutput(const char *msg) {
	_outputbuf += msg;
}


/**********************************************************************************************
 * handleConnection - performs a check of the connection, looking for data on the socket and
 *                    handling it based on the _status, or stage, of the connection
 *
 *		Returns: bytes read if connected, -1 if lost connection
 *
 *    Throws: socket_error for unexpected network issues
 **********************************************************************************************/

ssize_t TCPConn::handleConnection() {

	// If the client is not connected, then we're waiting for reconnect or timeout
   if (!isConnected()) {
		return -1;
	}

	ssize_t count = 0;
	std::string readbuf;

	// Receive data from the socket first
	while (_connfd.hasData()) {
		count += _connfd.readFD(readbuf);
		
		// Having data but not reading any data is a sign we lost connection
		if (readbuf.size() == 0) {
			disconnect();	
			return -1;
		}		

		_inputbuf += readbuf;
	}

	// Now write any data in the outputbuf to the connection
	_connfd.writeFD(_outputbuf);
	_outputbuf.clear();

	return count;
}


/**********************************************************************************************
 * disconnect - cleans up the socket as required and closes the FD
 *
 *    Throws: runtime_error for unrecoverable issues
 **********************************************************************************************/
void TCPConn::disconnect() {
   _connfd.closeFD();
	_is_connected = false;
}


/**********************************************************************************************
 * isConnected - performs a simple check on the socket to see if it is still open 
 *
 *    Throws: runtime_error for unrecoverable issues
 **********************************************************************************************/
bool TCPConn::isConnected() {
	return _is_connected;
}

/**********************************************************************************************
 * getIPAddrStr - gets a string format of the IP address and loads it in buf
 *
 **********************************************************************************************/
void TCPConn::getIPAddrStr(std::string &buf) {
   return _connfd.getIPAddrStr(buf);
}

/**********************************************************************************************
 * getUserInput - returns the data from the _inputbuf and clears that buffer
 *
 *		Returns: true if there is input, false if it's empty
 *
 **********************************************************************************************/
bool TCPConn::getUserInput(std::string &buf) {
	if (_inputbuf.size() == 0)
		return false;

   buf = _inputbuf;
	_inputbuf.clear();
	return true;
}


