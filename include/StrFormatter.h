#ifndef STRFORMATTER_H
#define STRFORMATTER_H

#include <stdexcept>
#include <map>
#include <string>

class format_error : public std::invalid_argument {
public:
   format_error(const std::string &what_arg):std::invalid_argument(what_arg) {}
   format_error(const char *what_arg):std::invalid_argument(what_arg) {}
};

class StrFormatter {
public:
	StrFormatter();
	~StrFormatter();

	bool addMap(const char mapping, const char *replstr);
	bool changeMap(const char mapping, const char *newstr);

	void formatStr(const char *inputstr, std::string &outputstr);

private:
	// replacement mapping
	std::map<char, std::string> _replmap;
};

#endif
