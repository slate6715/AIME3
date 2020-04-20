#ifndef TCPCONN_H
#define TCPCONN_H

#include <crypto++/secblock.h>
#include "FileDesc.h"
#include "LogMgr.h"

const int max_attempts = 2;

// Methods and attributes to manage a network connection, including tracking the username
// and a buffer for user input. Status tracks what "phase" of login the user is currently in
class TCPConn 
{
public:
   TCPConn(LogMgr &server_log, CryptoPP::SecByteBlock &key, unsigned int verbosity);
   ~TCPConn();

   // The current status of the connection
   enum statustype { s_none, s_connecting, s_connected, s_getauth, s_authresp,
                     s_auth, s_datatx, s_datarx, s_waitack, s_hasdata };

   statustype getStatus() { return _status; };

   bool accept(SocketFD &server);

   // Primary maintenance function. Checks this connection for input and handles it
   // depending on the state of the connection
   void handleConnection();

   // connect - second version uses ip_addr in network format (big endian)
   void connect(const char *ip_addr, unsigned short port);
   void connect(unsigned long ip_addr, unsigned short port);

   // Send data to the other end of the connection without encryption
   bool getData(std::vector<uint8_t> &buf);
   bool sendData(std::vector<uint8_t> &buf);

   // Calls encryptData or decryptData before send or after receive
   bool getEncryptedData(std::vector<uint8_t> &buf);
   bool sendEncryptedData(std::vector<uint8_t> &buf);

   // Simply encrypts or decrypts a buffer
   void encryptData(std::vector<uint8_t> &buf);
   void decryptData(std::vector<uint8_t> &buf);

   // Input data received on the socket
   bool isInputDataReady() { return _data_ready; };
   void getInputData(std::vector<uint8_t> &buf);

   // Data about the connection (NodeID = other end's Server Node ID string)
   unsigned long getIPAddr() { return _connfd.getIPAddr(); }; // Network format
   const char *getIPAddrStr(std::string &buf);
   unsigned short getPort() { return _connfd.getPort(); }; // host format
   const char *getNodeID() { return _node_id.c_str(); };

   // Connections can set the node or server ID of this connection
   void setNodeID(const char *new_id) { _node_id = new_id; };
   void setSvrID(const char *new_id) { _svr_id = new_id; };

   // Closes the socket
   void disconnect();

   // Checks if the socket FD is marked as open
   bool isConnected();

   // When should we try to reconnect (prevents spam)
   time_t reconnect;

   // Assign outgoing data and sets up the socket to manage the transmission
   void assignOutgoingData(std::vector<uint8_t> &data);

protected:
   // Functions to execute various stages of a connection 
   void sendSID();
   void waitForSID();
   void sendAuth();
   void checkForAuth(); 
   void checkForAuthResponse();
   void waitForData();
   void awaitAck();

   // Looks for commands in the data stream
   std::vector<uint8_t>::iterator findCmd(std::vector<uint8_t> &buf,
                                                   std::vector<uint8_t> &cmd);
   bool hasCmd(std::vector<uint8_t> &buf, std::vector<uint8_t> &cmd);

   // Gets the data between startcmd and endcmd strings and places in buf
   bool getCmdData(std::vector<uint8_t> &buf, std::vector<uint8_t> &startcmd,
                                                    std::vector<uint8_t> &endcmd);

   // Places startcmd and endcmd strings around the data in buf and returns it in buf
   void wrapCmd(std::vector<uint8_t> &buf, std::vector<uint8_t> &startcmd,
                                                    std::vector<uint8_t> &endcmd);


private:

   bool _connected = false;

   std::vector<uint8_t> c_rep, c_endrep, c_auth, c_endauth, c_ack, c_sid, c_endsid;

   statustype _status = s_none;

   SocketFD _connfd;
 
   std::string _node_id; // The username this connection is associated with
   std::string _svr_id;  // The server ID that hosts this connection object

   // Store incoming data to be read by the queue manager
   std::vector<uint8_t> _inputbuf;
   bool _data_ready;    // Is the input buffer full and data ready to be read?

   // Store outgoing data to be sent over the network
   std::vector<uint8_t> _outputbuf;

   CryptoPP::SecByteBlock &_aes_key; // Read from a file, our shared key
   std::string _authstr;   // remembers the random authorization string sent

   unsigned int _verbosity;

   LogMgr &_server_log;
};


#endif
