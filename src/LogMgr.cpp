#include <stdexcept>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <ostream>
#include <string>
#include <string.h>
#include "LogMgr.h"
#include "strfuncts.h"
#include "exceptions.h"


LogMgr::LogMgr():
				_log_file(""),
				_log_lvl(1),
				_lfptr(NULL)
{
}
				

// Log manager, supports log_lvl for verbosity control
LogMgr::LogMgr(const char *log_file, unsigned int log_lvl):
											_log_file(log_file),
											_log_lvl(log_lvl),
											_lfptr(NULL)
{

}


LogMgr::~LogMgr() {
   closeLog();
}

LogMgr::LogMgr(const LogMgr &copy_from):
										_log_file(copy_from._log_file),
										_log_lvl(copy_from._log_lvl),
										_lfptr(NULL)
{
}


LogMgr &LogMgr::operator = (const LogMgr &copy_from) {
	_log_file = copy_from._log_file;
	_log_lvl = copy_from._log_lvl;
	_lfptr = NULL;
	return *this;
}

/***************************************************************************************************
 * createTimeStamp - creates a time stamp string and places it in buf
 ***************************************************************************************************/
void LogMgr::createTimestamp(std::string &buf) {
   // Put together our timestamp and start the log with the stamp
   std::string logstr;
   time_t curtime = time(NULL);
   char timestr[27];
   if (ctime_r(&curtime, timestr) == NULL)
      throw std::runtime_error("ctime_r function failed unexpectedly");

   buf = timestr;
   clrNewlines(buf);

}

/***************************************************************************************************
 * writeLog - Writes a string to a log with the timestamp
 *
 *    Params:  str - string to write to the log in const char * or std::string format
 *             lvl - the "importance" of this log - can be used to set verbosity
 ***************************************************************************************************/

void LogMgr::writeLog(const char *str, unsigned int lvl) {

   // Don't log it if the item is not important enough for logging verbosity
   if (lvl > _log_lvl)
      return;

   // If the file is not open yet, open it
   if (_lfptr == NULL) {
      if ((_lfptr = fopen(_log_file.c_str(), "a+")) == NULL) {
			throw logfile_error("Unable to open log file to append.");
      }
   }

   // Put together our timestamp and start the log with the stamp
   std::string logstr;
   createTimestamp(logstr);

   // Now add on the text to log
   logstr += " ";
   logstr += str;
   logstr += "\n";

   fputs(logstr.c_str(), _lfptr);
   fflush(_lfptr);

}

void LogMgr::writeLog(std::string &str, unsigned int lvl) {
   return writeLog(str.c_str(), lvl);
}

/***************************************************************************************************
 * strerrLog - Writes to the log and retrieves the errno string to attach at the end
 *
 *    Params:  str - string to write to the log in const char * or std::string format
 *             lvl - the "importance" of this log - can be used to set verbosity
 ***************************************************************************************************/

void LogMgr::strerrLog(const char *str, unsigned int lvl) {
   char strerr_buf[100];
   std::string logstr = str;

   logstr += " Reason: ";
   if (strerror_r(errno, strerr_buf, 100) != 0) 
      throw std::runtime_error("Unexpected error writing to log when calling strerror_r");

   logstr += strerr_buf;
   return writeLog(logstr.c_str(), lvl);
}

// self-explanatory
void LogMgr::closeLog() {
   if (_lfptr != NULL) {
      fclose(_lfptr);
      _lfptr = NULL;
   }
}


/***************************************************************************************************
 * changeFilename - Changes the filename the log file is set to write to
 *
 ***************************************************************************************************/

void LogMgr::changeFilename(const char *filename) {
   closeLog();
   _log_file = filename;
}
