
#include <sti/utils/ConfigFile.h>
#include <sti/utils/utils.h>

#include <fstream>
#include <vector>
#include <iostream>

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

	if (!configFile.is_open()) {
		parsed = false;
		//std::cerr << "Error opening config file '" << filename_ << "'." << std::endl;
		return;
	}

	std::string line;
	bool success = true;
	std::size_t commentLoc, sectionHeadStart, sectionHeadEnd, equalsLoc;
	std::string section = "";	//default section is blank
	parsed = true;	//unless there's a problem

	while (success && getline(configFile, line))
	{
		equalsLoc = line.find_first_of("=");
		sectionHeadStart = line.find_first_of("[");

		//Sections are written as [...] with no preceeding = sign
		if (sectionHeadStart != std::string::npos && equalsLoc == std::string::npos) {
			//new section found
			sectionHeadEnd = line.find_first_of("]");
			auto nextSection = line.substr(sectionHeadStart + 1, sectionHeadEnd - sectionHeadStart - 1);

			//check for relative subsection
			auto found = nextSection.find_first_of(".");
			if (found != std::string::npos && found == 0) {	//first character is a dot, like [.subsection]
				nextSection = section + nextSection;	//convert to absolute subsection name
			}

			section = nextSection;
		}
		else {
			commentLoc = line.find_first_of("#");
			success = assignStringValue(section, line.substr(0, commentLoc));
		}

	}

	if (!success) {
		std::cerr << "Error parsing config file '" << filename_ << "' at line" << std::endl
			<< ">>> " << line << std::endl;
	}

	parsed = success;
	configFile.close();
}


bool ConfigFile::assignStringValue(const std::string& section, std::string line)
{
	
	std::size_t nameStart = line.find_first_not_of(" ");

	if (line.length() == 0 || nameStart == std::string::npos) {		//blank line
		return true;
	}

	std::size_t equalsLoc = line.find_first_of("=");

	if (equalsLoc == std::string::npos) {
		//Missing equals sign
		return false;
	}

	std::size_t nameEnd = line.find_last_not_of(" ", equalsLoc - 1);

	if (equalsLoc == line.length() - 1) {
		line.append("");
	}

	std::size_t valueStart = line.find_first_not_of(" ", equalsLoc + 1);

	if (valueStart == std::string::npos) {
		valueStart = equalsLoc + 1;
	}

	if (equalsLoc == nameStart) {
		if (includes(section, lastParsedName)) {
			//Found equals sign (with no key name) below another valid entry
			//appending new value to previous entry as list
			addToList(section, lastParsedName, line.substr(valueStart));			
		}
		else {
			//can only addToList to an existing entry
			return false;
		}
	}
	else {
		lastParsedName = STI::Utils::trim( line.substr(nameStart, nameEnd + 1) );
		set(section, lastParsedName, line.substr(valueStart));		
	}

	return true;
}

