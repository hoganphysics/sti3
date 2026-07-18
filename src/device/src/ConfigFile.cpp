
#include <sti/utils/ConfigFile.h>
#include <sti/utils/utils.h>

#include <fstream>
#include <sstream>
#include <vector>
#include <iostream>

using STI::Utils::ConfigFile;

namespace {

std::string removeInlineComment(const std::string& line)
{
	bool inSingleQuote = false;
	bool inDoubleQuote = false;
	bool escaped = false;

	for (std::size_t i = 0; i < line.size(); ++i) {
		const auto c = line[i];

		if (escaped) {
			escaped = false;
			continue;
		}

		if (c == '\\') {
			escaped = true;
			continue;
		}

		if (c == '\'' && !inDoubleQuote) {
			inSingleQuote = !inSingleQuote;
			continue;
		}

		if (c == '"' && !inSingleQuote) {
			inDoubleQuote = !inDoubleQuote;
			continue;
		}

		if (c == '#' && !inSingleQuote && !inDoubleQuote) {
			return line.substr(0, i);
		}
	}

	return line;
}

bool isSupportedEscape(char c)
{
	return c == '#' || c == '"' || c == '\'' || c == '\\';
}

std::string unescapeValue(const std::string& value)
{
	std::string unescaped;
	bool escaped = false;

	for (const auto c : value) {

		if (escaped) {
			if (isSupportedEscape(c)) {
				unescaped.push_back(c);
			}
			else {
				unescaped.push_back('\\');
				unescaped.push_back(c);
			}
			escaped = false;
			continue;
		}

		if (c == '\\') {
			escaped = true;
			continue;
		}

		unescaped.push_back(c);
	}

	if (escaped) {
		unescaped.push_back('\\');
	}

	return unescaped;
}

std::string parseEscapedValue(std::string value)
{
	value = STI::Utils::trim(value);

	if (value.size() >= 2) {
		const auto quote = value.front();
		if ((quote == '"' || quote == '\'') && value.back() == quote) {
			return unescapeValue(value.substr(1, value.size() - 2));
		}
	}

	return unescapeValue(value);
}

bool hasMatchingOuterQuotes(const std::string& value)
{
	if (value.size() < 2) {
		return false;
	}

	const auto quote = value.front();
	return (quote == '"' || quote == '\'') && value.back() == quote;
}

std::string quoteForSave(const std::string& value)
{
	if (value.find_first_of("#\\\"") == std::string::npos && !hasMatchingOuterQuotes(value)) {
		return value;
	}

	std::string quoted;
	quoted.reserve(value.size() + 2);
	quoted.push_back('"');

	for (const auto c : value) {
		if (c == '\\' || c == '"') {
			quoted.push_back('\\');
		}
		quoted.push_back(c);
	}

	quoted.push_back('"');
	return quoted;
}

} // namespace


ConfigFile::ConfigFile()
: ConfigFile("")
{
}

ConfigFile::ConfigFile(const std::string& filename)
: Configuration(), filename_(filename), parsed(false)
{
	load(filename_);
}

void ConfigFile::load(const std::string& filename, bool autocreate)
{
	filename_ = filename;
	clear();	//clear existing data if this is a reload
	load(autocreate);
}

void ConfigFile::load(bool autocreate)
{
	if (autocreate) {
		//check if file exists, if not create it
		std::ifstream file(filename_.c_str());
		if (!file.is_open()) {
			std::ofstream newFile(filename_.c_str());
			if (!newFile.is_open()) {
				std::cerr << "Error creating config file '" << filename_ << "'." << std::endl;
				parsed = false;
				return;
			}
			newFile.close();
		}
		else {
			file.close();
		}
	}
	
	std::fstream configFile(filename_.c_str(), std::fstream::in);

	if (!configFile.is_open()) {
		parsed = false;
		//std::cerr << "Error opening config file '" << filename_ << "'." << std::endl;
		return;
	}

	std::string line;
	bool success = true;
	std::size_t sectionHeadStart, sectionHeadEnd, equalsLoc;
	std::string section = "";	//default section is blank
	parsed = true;	//unless there's a problem

	while (success && getline(configFile, line))
	{

		std::string lineNoComment = removeInlineComment(line);

		equalsLoc = lineNoComment.find_first_of("=");
		sectionHeadStart = lineNoComment.find_first_of("[");

		//Sections are written as [...] with no preceeding = sign
		if (sectionHeadStart != std::string::npos && equalsLoc == std::string::npos) {
			//new section found
			sectionHeadEnd = lineNoComment.find_first_of("]");
			
			std::string sectionNameBody;
			if (sectionHeadEnd != std::string::npos) {
				sectionNameBody = lineNoComment.substr(sectionHeadStart + 1,
					sectionHeadEnd - sectionHeadStart - 1);
			} else {
				sectionNameBody = lineNoComment.substr(sectionHeadStart + 1);
			}

			auto nextSection = STI::Utils::trim(sectionNameBody);

			//check for relative subsection
			auto found = nextSection.find_first_of(".");
			if (found != std::string::npos && found == 0) {	//first character is a dot, like [.subsection]
				nextSection = section + nextSection;	//convert to absolute subsection name
			}

			section = nextSection;
		}
		else {
			success = assignStringValue(section, lineNoComment);
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

	auto value = parseEscapedValue(line.substr(valueStart));

	if (equalsLoc == nameStart) {
		if (includes(section, lastParsedName)) {
			//Found equals sign (with no key name) below another valid entry
			//appending new value to previous entry as list
			addToList(section, lastParsedName, value);
		}
		else {
			//can only addToList to an existing entry
			return false;
		}
	}
	else {
		lastParsedName = STI::Utils::trim( line.substr(nameStart, nameEnd + 1) );
		set(section, lastParsedName, value);
	}

	return true;
}

void ConfigFile::setHeader(const std::string& header)
{
	headerComment = header;
}

void ConfigFile::save()
{
	std::fstream configFile(filename_.c_str(), std::fstream::out);
	
	if (!configFile.is_open()) return;
	
	//write header in comment block
	std::stringstream header(headerComment);
	for (std::string line; getline(header, line, '\n');) {
        configFile << "# " << line << std::endl;
    }
	configFile << std::endl;

	//write parameters in all sections
	for (auto& section : configData) {

		if (section.first.compare("") != 0) {	//skip if no section name
			configFile << "[" << section.first << "]" << std::endl;
		}

		for (auto& parameter : section.second.parameters) {
			configFile << parameter.first << " = " << quoteForSave(parameter.second) << std::endl;
		}
		configFile << std::endl;
	}

	configFile.close();
}
