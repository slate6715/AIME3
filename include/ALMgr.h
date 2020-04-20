#ifndef ALMGR_H
#define ALMGR_H

#include <string>

/********************************************************************************
 * ALMgr - Access List manager, basically reads from a text document to find the
 *         IP address given. If it's a whitelist, then returns true for allowed
 *         if found and opposite for blacklists
 ********************************************************************************/

class ALMgr {
   public:
      ALMgr(const char *al_file, bool is_whitelist = true);
      ~ALMgr();

      bool isAllowed(const char *ipaddr);
      bool isAllowed(unsigned long ipaddr);

   private:
      std::string _al_file;

      bool _is_whitelist;
};

#endif // ALMGR_H
