
#ifndef STI_DEVICE_CONFIGFILE_H
#define STI_DEVICE_CONFIGFILE_H

#include "Configuration.h"

#include <map>
#include <string>
#include <sstream>
#include <vector>


namespace STI
{

namespace Device
{

class ConfigFile : public Configuration
{
public:

    ConfigFile();
	ConfigFile(const std::string& filename);
	~ConfigFile() {}

	void parse(const std::string& filename);
	bool isParsed() const { return parsed; }

private:
	
	bool assignStringValue(const std::string& section, std::string line);

	std::string filename_;
	bool parsed;
};


} //Device
} //STI

#endif

