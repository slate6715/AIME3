#include <stdexcept>
#include <strings.h>
#include <fcntl.h>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>

#include "FileDesc.h"
#include "strfuncts.h"

const unsigned int bufsize = 500;

FileDesc::FileDesc() {

}


FileDesc::~FileDesc() {

}

/*****************************************************************************************
 * setNonBlocking - marks the FD as non-blocking, meaning that certain calls that would
 *                  normally block while looking for data will return immediately if no
 *                  data is found.
 *
 *****************************************************************************************/

void FileDesc::setNonBlocking() {
   // Set the socket to nonblocking
   int flags = fcntl(_fd, F_GETFL);
   if ((flags < 0) || (fcntl(_fd, F_SETFL, flags | O_NONBLOCK) < 0)) {
      throw socket_error("Failed setting file descriptor to nonblocking.");
   }

}

/*****************************************************************************************
 * writeByte - writes a single byte to the FD
 *
 *    Params: data - the byte of data to write
 *
 *    Returns: returns the results of the FD write function, 1 for success, -1 for failure
 *****************************************************************************************/
ssize_t FileDesc::writeByte(unsigned char data) {
   return write(_fd, &data, 1);
}

/*****************************************************************************************
 * writeByte - reads a single byte from the FD
 *
 *    Params: buf - single unsigned char to store the read-in byte
 *
 *    Returns: returns the results of the FD read function, 1 for success, -1 for failure
 *****************************************************************************************/

ssize_t FileDesc::readByte(unsigned char &buf) {
   return read(_fd, &buf, 1);
}

/*****************************************************************************************
 * hasData - uses the select function to poll the FD for available read data
 *
 *    Params: ms_timeout - milliseconds to wait for data before returning if none found
 *
 *    Returns: true if data is available for reading, false otherwise
 *****************************************************************************************/

bool FileDesc::hasData(long ms_timeout) {
   fd_set read_fds;
   timeval timeout;

   timeout.tv_sec = 0;
   timeout.tv_usec = ms_timeout;

   FD_ZERO(&read_fds);
   FD_SET(_fd, &read_fds);

   int n;
   if ((n = select(_fd+1, &read_fds, NULL, NULL, &timeout)) == -1) {
      throw socket_error("Select error on file descriptor.");
   }

   if (n == 0)
      return false;
   return true;
}

/*****************************************************************************************
 * readFD - simply reads all available string data (up to bufsize) from the FD
 *
 *    Params: buf - string to store the data in
 *
 *    Returns: returns the amount of data read or -1 for failure
 *****************************************************************************************/

ssize_t FileDesc::readFD(std::string &buf) {
   char *readbuf = new char[bufsize];
   bzero(readbuf, sizeof(char) * bufsize);
   ssize_t amt_read = 0;
   if ((amt_read = read(_fd, readbuf, bufsize)) < 0) {
      delete readbuf;
      return -1;
   }
   
   buf = readbuf;
   delete readbuf;
   return amt_read;
}

/*****************************************************************************************
 * writeFD - writes all the string data provided in str to the FD
 *
 *    Params: str/data - the string data to write to the FD
 *
 *    Returns: returns the amount written for success, -1 for failure
 *****************************************************************************************/

ssize_t FileDesc::writeFD(std::string &str) {
   return writeFD(str.c_str(), str.size());
}

ssize_t FileDesc::writeFD(const char *data) {
   return writeFD(data, strlen(data));
}

ssize_t FileDesc::writeFD(const char *data, unsigned int len) {
   return write(_fd, data, len);
}

/*************************************************************************************
 * isOpen - determines if the file descriptor is open for both reading and writing
 *          
 *    Returns: true if open for both, false if down for either
 *
 *************************************************************************************/
bool FileDesc::isOpen() {
   if ((fcntl(_fd, F_GETFD) == -1) && (errno == EBADF))
      return false;
   return true;
}

/***************************************************************************************
 * closeFD - closes the FD cleanly
 ***************************************************************************************/
void FileDesc::closeFD() {
   close(_fd);
}

/****************************************************************************************
 * SocketFD (constructor) - Creates the socket FD for network sockets
 *
 *    Throws: socket_error if the socket creation function fails for some reason
   
 ****************************************************************************************/

SocketFD::SocketFD():FileDesc() {

   // Create the socket
   _fd = socket(AF_INET, SOCK_STREAM, 0);
   if (_fd == -1) {
      throw socket_error("Socket creation failed.");
   }
   bzero(&_fd_addr, sizeof(_fd_addr));
}

SocketFD::~SocketFD() {

}

/*****************************************************************************************
 * setReusable - sets the socket so the address can be reused. Gets rid of the annoying
 *               can't open socket errors upon it not being closed properly
 *
 *****************************************************************************************/

void SocketFD::setReusable() {
   
   int enable = 1;
   if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) < 0)
      throw socket_error("setsockopt failure setting SO_REUSEADDR");

}

/*****************************************************************************************
 * bindFD - Binds the FD to the given network ip address and port, making it available to
 *          accept connections.
 *
 *    Params: ip_addr - the IP address string of the server in standard format
 *            port - the port number to bind to
 *
 *    Throws: socket_error for issues binding the socket
 *****************************************************************************************/

void SocketFD::bindFD(const char *ip_addr, short unsigned int port) {

   // Load the socket information to prep for binding
   _fd_addr.sin_family = AF_INET;
   inet_pton(AF_INET, ip_addr, &_fd_addr.sin_addr.s_addr);
   _fd_addr.sin_port = htons(port);

   if ((bind(_fd, (struct sockaddr *) &_fd_addr, sizeof(_fd_addr))) != 0) {
      throw socket_error("Socket bind failed.");
   }

}

/*****************************************************************************************
 * connectTo - attempts to connect via TCP to the given ip address and port
 *
 *    Params:  ip_addr - the IP address string of the server to connect to in std format
 *             port - the port of the server to connect to
 *
 *    Returns: true if the connect worked, false otherwise
 *****************************************************************************************/

bool SocketFD::connectTo(const char *ip_addr, unsigned short port) {

   unsigned long n_ip_addr;

   inet_pton(AF_INET, ip_addr, &n_ip_addr);
   return connectTo(n_ip_addr, htons(port));
}

bool SocketFD::connectTo(unsigned long ip_addr, unsigned short port) {
   if ((_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
      throw socket_error("Socket creation failed.");

   // Load the socket information to prep for binding
   bzero(&_fd_addr, sizeof(_fd_addr));
   _fd_addr.sin_family = AF_INET;
   _fd_addr.sin_addr.s_addr = ip_addr;
   _fd_addr.sin_port = port;

   if (connect(_fd, (struct sockaddr *) &_fd_addr, sizeof(_fd_addr)) != 0)
      return false;

   return true;

}

/*****************************************************************************************
 * listenFD - starts listening for connections on a bound socket FD
 *
 *    Params: backlog - the number of concurrent connections to allow into the queue awaiting
 *                      the accept function
 *
 *    Throws: socket_error if issues arise with the listen function
 *****************************************************************************************/

void SocketFD::listenFD(int backlog) {
   if (listen(_fd, backlog) != 0)
      throw socket_error("Server failed attempting to listen on port");
}


/*****************************************************************************************
 * acceptFD - Given a passed-in server FD, accepts a connection and assigns to THIS FD
 *
 *    Params: server - a bound, listening server FD that has an available connection
 *
 *    Returns: false if the accept failed, true otherwise
 *****************************************************************************************/

bool SocketFD::acceptFD(SocketFD &server) {
   socklen_t len = sizeof(_fd_addr);

   _fd = accept(server.getFD(), (struct sockaddr *) &_fd_addr, &len);
   if (_fd == -1)
      return false;

   return true;
}

/*****************************************************************************************
 * getIPAddr - returns the IP address of this FD in big endian format
 *
 *****************************************************************************************/

unsigned long SocketFD::getIPAddr() {
   return _fd_addr.sin_addr.s_addr;
}

/******************************************************************************************
 * getPort - gets the port of this socket in host byte order (little endian, or regular int)
 *
 ******************************************************************************************/
unsigned short SocketFD::getPort() {
   return ntohs(_fd_addr.sin_port);
}


/******************************************************************************************
 * getIPAddrStr - gets a string format of the IP address and loads it in buf
 *
 ******************************************************************************************/
void SocketFD::getIPAddrStr(std::string &buf) {
   char ipaddr_str[16];
   inet_ntop(AF_INET, (void *) &_fd_addr.sin_addr.s_addr, ipaddr_str, 16);
   buf = ipaddr_str;
}

TermFD::TermFD():FileDesc() {
   _fd = 0;
}

TermFD::~TermFD() {

}

/*****************************************************************************************
 * setEchoFD - Turns off and on the local echo. When local echo is off, users can't see
 *             what they are typing on the screen
 *
 *    Params: echo - true to make what the user types visible, false invisible
 *****************************************************************************************/

void TermFD::setEchoFD(bool echo) {
   hideInput(_fd, !echo);
}


FileFD::FileFD(const char *filename):FileDesc(), _filename(filename) {

}

FileFD::~FileFD() {

}

/******************************************************************************************
 * openFile - opens a file for reading or writing/appending
 *
 *    Params:  ftype - the type FD - options are:
 *                   readfd - read only
 *                   writefd - write only
 *                   appendfd - write only, moves pointer to the end
 *             create - if the file doesn't exist, setting this true will cause it to be
 *                      created
 *
 *    Returns: false if the file failed to open, true otherwise
 *
 ******************************************************************************************/

bool FileFD::openFile(fd_file_type ftype, bool create) {
   int file_flags[] = {O_RDONLY, O_WRONLY, O_WRONLY | O_APPEND};

   int flags = file_flags[ftype];
   if (create)
      flags |= O_CREAT;

   if ((_fd = open(_filename.c_str(), flags, S_IRUSR | S_IWUSR)) == -1)
      return false;

   return true;
}

/*****************************************************************************************
 * readStr - For a file FD, reads in characters until it hits a newline char. Not set up to
 *          work with sockets as it does not buffer and could lose data if partial data
 *          arrives.
 *
 *    Params:  buf - the STL string buf to put the results into
 *
 *    Returns: number of bytes read, or -1 for error
 *
 *****************************************************************************************/

ssize_t FileDesc::readStr(std::string &buf) {
   char strbuf[100];
   unsigned int i = 0;
   char readchar = 0;
   int results;

   buf.clear();

   while ((results = read(_fd, &readchar, 1) > 0) && (readchar != '\n')) {
      strbuf[i++] = readchar;

      // If we're overflowing our buffer, dump into the std::string and clear the buffer
      if (i == 99) {
         strbuf[99] = '\0';
         buf += strbuf;
         i = 0;
      }
   }

   if (results == -1)
      return -1;

   strbuf[i] = '\0';
   buf += strbuf;
   return buf.size();
}

