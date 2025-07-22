
#ifndef STI_UTILS_CONFIGFILE_H
#define STI_UTILS_CONFIGFILE_H

#include <sti/utils/Configuration.h>

#include <map>
#include <string>
#include <sstream>
#include <vector>


namespace STI
{

namespace Utils
{

class ConfigFile : public Configuration
{
public:

    ConfigFile();
	ConfigFile(const std::string& filename);
	~ConfigFile() {}

	void load(bool autocreate = false);
	void save();

	void load(const std::string& filename, bool autocreate = false);
	bool isParsed() const { return parsed; }

	void setHeader(const std::string& header);

private:
	
	bool assignStringValue(const std::string& section, std::string line);

	std::string filename_;
	std::string headerComment;
	bool parsed;
	std::string lastParsedName;
};


} //Utils
} //STI

#endif

