#include <sstream>
#include "StrFormatter.h"

StrFormatter::StrFormatter() {

}


StrFormatter::~StrFormatter() {

}

/*********************************************************************************************
 * addMap - adds an entry to the map
 *
 *		Returns: true if successful, false if it is already there
 *
 *********************************************************************************************/

bool StrFormatter::addMap(const char mapping, const char *replstr) {
	auto m_it = _replmap.insert(std::pair<char, std::string>(mapping, std::string(replstr)));

	return m_it.second;
}

/*********************************************************************************************
 * changeMap - find a map entry and changes the text associated with it
 *
 *    Returns: true if successful, false if it was not found
 *
 *********************************************************************************************/

bool StrFormatter::changeMap(const char mapping, const char *newstr) {
	auto m_it = _replmap.find(mapping);

	if (m_it == _replmap.end())
		return false;

	m_it->second = newstr;
	return true;
}

/*********************************************************************************************
 * formatStr - takes the input string and replaces the % maps with the replacement string 
 *
 *
 *********************************************************************************************/

void StrFormatter::formatStr(const char *inputstr, std::string &outputstr) {
   std::stringstream errmsg;
	std::string input = inputstr;
   outputstr.clear();

   size_t pos;
   size_t start = 0;
   std::string name;

   // Find the next %
   while ((pos = input.find("%", start)) != std::string::npos) {
      // Add all before the % to the buffer
      outputstr += input.substr(start, pos-start);
      pos++;

		// The odd situation of a % right before the string ends
      if ((pos >= input.size()) || (!isalnum(input[pos]))) {
			throw format_error("% without valid character following it");
      }

		auto m_it = _replmap.find(input[pos]);
		if (m_it == _replmap.end()) {
			errmsg << "% followed by unknown mapping '" << input[pos+1] << "'";
			throw format_error(errmsg.str().c_str());
		}
		
		outputstr += m_it->second;
		start = pos + 1;
	} 	

	if (start < input.size()) {
		outputstr += input.substr(start, input.size()-start);
	}
}

