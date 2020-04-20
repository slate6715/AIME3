#ifndef FILEDESC_H
#define FILEDESC_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <vector>
#include <unistd.h>
#include "exceptions.h"

// Manages File Descriptors by largely simplfying their interfaces for specific purposes.
// FileDesc provides some limited functionality and could be instantiated, but child
// classes may provide specialized capability. These include:
// 
// SocketFD - Network socket FD with stored IP/port information in sockaddr_in
// TermFD - Stdin terminal
// FileFD - non-buffered file FD with ability to write/read binary data

class FileDesc
{
public:
   FileDesc();
   virtual ~FileDesc();

   // Ensures certain functions do not block
   void setNonBlocking();

   // Basic write function to write data to the FD
   ssize_t writeFD(std::string &str);
   ssize_t writeFD(const char *data);
   ssize_t writeFD(const char *data, unsigned int len);

   // Basic read function to read all string data off the FD
   ssize_t readFD(std::string &buf);

   // Reads one character from the buffer at a time until it finds a newline
   ssize_t readStr(std::string &buf);

   // Read a single byte from the FD
   ssize_t readByte(unsigned char &buf);

   // Writes a single byte to the FD
   ssize_t writeByte(unsigned char data);

   // Checks if the FD has data available to be read
   bool hasData(long ms_timeout = 10);

   // Checks if the FD is still open (network connections will still appear open even if lost link)
   bool isOpen();

   int getFD() { return _fd; };

   void closeFD();

   // The code must be defined here for a template for the next two functions
   /*****************************************************************************************
    * readBytes - Template method--for an FD, reads in sizeof(T) * n bytes and stores in a
    *             vector of type T
    *
    *    Params:  buf - the STL vector to store the bytes
    *
    *    Returns: number of bytes read, or -1 for read error, -2 if not enough bytes
    *             were available to fill a complete set of size T variables
    *
    *****************************************************************************************/

   template <typename T>
   int readBytes(std::vector<T> &buf, int n) {
      int datasize = sizeof(T);
      int bufsize = datasize * n;

      unsigned char *bytebuf = new unsigned char[bufsize];

      buf.clear();

      int results;
      if ((results = read(_fd, bytebuf, bufsize)) < 0)
      {
         delete bytebuf;
         return -1;
      }

      if (results < bufsize) {
         if (results % datasize != 0) {
            delete bytebuf;
            return -2;
         }
      }

      unsigned int read_units = results / datasize;
      buf.reserve(read_units);
      for (unsigned int i=0; i< read_units; i++) {
         buf.push_back((T) bytebuf[i*datasize]);
      }

      delete bytebuf;
      return buf.size();
   }

   /*****************************************************************************************
    * writeBytes - Template method--takes a STL vector object of type T and writes raw bytes to
    *              the FD
    *
    *    Params:  buf - the STL vector to store the bytes
    *
    *    Returns: number of bytes read, or -1 for read error, -2 if not enough bytes
    *             were available to fill a complete set of size T variables
    *
    *****************************************************************************************/

   template <typename T>
   int writeBytes(std::vector<T> &buf) {
      int datasize = sizeof(T);
      int bufsize = datasize * buf.size();

      unsigned char *bytebuf = new unsigned char[bufsize];
      for (unsigned int i=0; i<buf.size(); i++) {
         memcpy(&bytebuf[i*datasize], &buf[i], sizeof(T));
      }

      int results;
      results = write(_fd, bytebuf, bufsize);
      delete bytebuf;
      return results;

   }


 
protected:

   int _fd;
 
};

/********************************************************************************************
 * SocketFD class - includes methods for managing a network socket
 *
 ********************************************************************************************/

class SocketFD : public FileDesc {
public:
   SocketFD();
   ~SocketFD();

   void bindFD(const char *ip_addr, unsigned short int port);
   bool connectTo(const char *ip_addr, unsigned short port);
   bool connectTo(unsigned long ip_addr, unsigned short port);
   void listenFD(int backlog = 5);
   bool acceptFD(SocketFD &server);

   // Sets this address to reusable to prevent problems when sockets don't shut down properly
   void setReusable();

   unsigned long getIPAddr();  // Gets IP in big endian (network) format
   void getIPAddrStr(std::string &buf); // The IP string associated with this socket
   unsigned short getPort();   // Port in little-endian (host) format

private:

   sockaddr_in _fd_addr;

};

/********************************************************************************************
 * TermFD class - includes methods for reading from stdin and writing to stdout and
 *                configuring them as required 
 *
 ********************************************************************************************/

class TermFD : public FileDesc {
public:
   TermFD();
   ~TermFD();

   void setEchoFD(bool echo);

private:

};

/********************************************************************************************
 * FileFD class - includes methods for reading from and writing to a file.
 *
 ********************************************************************************************/

class FileFD : public FileDesc {
public:
   FileFD(const char *filename);
   ~FileFD();

   enum fd_file_type {readfd, writefd, appendfd};

   bool openFile(fd_file_type ftype, bool create = false);

private:
   std::string _filename; 
};


#endif
