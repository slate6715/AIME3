#include <arpa/inet.h>
#include <stdexcept>
#include "ALMgr.h"
#include "strfuncts.h"

ALMgr::ALMgr(const char *al_file, bool is_whitelist):_al_file(al_file),_is_whitelist(is_whitelist) {

}


ALMgr::~ALMgr() {

}

/******************************************************************************************************
 * isAllowed - checks to see if the IP address is in the list and allows/denies based off _is_whitelist
 *  
 *    Second version takes in an unsigned long IP Addr in network (big endian) format
 ******************************************************************************************************/
bool ALMgr::isAllowed(const char *ipaddr) {
   in_addr testaddr;

   inet_pton(AF_INET, ipaddr, &testaddr);
   return isAllowed(testaddr.s_addr);
}

bool ALMgr::isAllowed(unsigned long ipaddr) {
   FILE *alfile;

   // First, find the IP address in the file
   if ((alfile = fopen(_al_file.c_str(), "r")) == NULL) {
      throw std::runtime_error("Unable to open white list file.");
   }

   in_addr al_ip;
   char strbuf[30];
   strbuf[0] = '\0';
   std::string ipstr;

   while (fgets(strbuf, 30, alfile) != NULL) {
      ipstr = strbuf;
      clrNewlines(ipstr);
      inet_pton(AF_INET, ipstr.c_str(), &al_ip);

      if (al_ip.s_addr == ipaddr) {
         fclose(alfile);
         if (_is_whitelist)
            return true;
         else
            return false;
      }
   }

   fclose(alfile);

   if (_is_whitelist)
      return false;
   return true;
}

