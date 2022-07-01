
#include <sti/utils/ConfigFile.h>
#include <sti/utils/utils.h>

#include <fstream>
#include <vector>

using STI::Utils::ConfigFile;


ConfigFile::ConfigFile()
: ConfigFile("")
{
}

ConfigFile::ConfigFile(const std::string& filename)
: Configuration(), filename_(filename), parsed(false)
{
	parse(filename_);
}

void ConfigFile::parse(const std::string& filename)
{
	std::fstream configFile(filename.c_str(), std::fstream::in);

	if (!configFile.is_open())
	{
		parsed = false;
		//std::cerr << "Error opening config file '" << filename_ << "'." << std::endl;
		return;
	}

	std::string line;
	bool success = true;
	std::size_t commentLoc, sectionHeadStart, sectionHeadEnd;
	std::string section = "";	//default section is blank
	parsed = true;	//unless there's a problem

	while (success && getline(configFile, line))
	{
		sectionHeadStart = line.find_first_of("[");
		if (sectionHeadStart != std::string::npos) {
			//new section found
			sectionHeadEnd = line.find_first_of("]");
			section = line.substr(sectionHeadStart + 1, sectionHeadEnd - 1);
		}
		else {
			commentLoc = line.find_first_of("#");
			success = assignStringValue(section, line.substr(0, commentLoc));
		}

	}

	if (!success)
	{
//		std::cerr << "Error parsing config file '" << filename_ << "' at line" << std::endl
//			<< ">>> " << line << std::endl;
	}

	parsed = success;
	configFile.close();
}


bool ConfigFile::assignStringValue(const std::string& section, std::string line)
{
	
	std::size_t nameStart = line.find_first_not_of(" ");

	if (line.length() == 0 || nameStart == std::string::npos)	//blank line
		return true;

	std::size_t equalsLoc = line.find_first_of("=");

	if (equalsLoc < 1 || equalsLoc == std::string::npos || equalsLoc == nameStart)
		return false;

	std::size_t nameEnd = line.find_last_not_of(" ", equalsLoc - 1);

	if (equalsLoc == line.length() - 1)
		line.append("");

	std::size_t valueStart = line.find_first_not_of(" ", equalsLoc + 1);

	if (valueStart == std::string::npos)
		valueStart = equalsLoc + 1;

	set(section, line.substr(nameStart, nameEnd + 1), line.substr(valueStart));
	return true;
}

