#ifndef LOGMGR_H
#define LOGMGR_H

#include <string>

/********************************************************************************
 * LogMgr - Log file manager. Includes setting log levels and a function to write
 *          a log entry if it is below a specified log level. Opens for each write
 *          in append mode to help with thread conflicts (but no mutexing)
 ********************************************************************************/

class LogMgr {
   public:
		LogMgr();
      LogMgr(const char *log_file, unsigned int log_lvl);
		LogMgr(const LogMgr &copy_from);
      ~LogMgr();

		LogMgr &operator = (const LogMgr &copy_from);

      void writeLog(const char *str, unsigned int lvl=0);
      void writeLog(std::string &str, unsigned int lvl=0);
      void strerrLog(const char *str, unsigned int lvl=0);

      void closeLog();
      
      unsigned int getLogLvl() { return _log_lvl; };
		void setLogLvl(unsigned int new_lvl) { _log_lvl = new_lvl; };

      static void createTimestamp(std::string &buf);

      void changeFilename(const char *filename);

   private:
      std::string _log_file;  // Path/name of the log to write to
      unsigned int _log_lvl;  // The verbosity level
   
      FILE *_lfptr = NULL;
};

#endif // ALMGR_H
