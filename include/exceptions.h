#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <stdexcept>

// Designed to be used for an error binding or opening a network connection, etc
class socket_error : public std::runtime_error
{
public:
      socket_error( const std::string &what_arg):runtime_error(what_arg){};
      socket_error( const char *what_arg):runtime_error(what_arg){};
};

// An issue exists with the password file
class pwfile_error : public std::runtime_error {
public:
   pwfile_error(const std::string &what_arg):runtime_error(what_arg) {}
   pwfile_error(const char *what_arg):runtime_error(what_arg) {}
};

class logfile_error : public std::runtime_error {
public:
   logfile_error(const std::string &what_arg):runtime_error(what_arg) {}
   logfile_error(const char *what_arg):runtime_error(what_arg) {}   
};

#endif
