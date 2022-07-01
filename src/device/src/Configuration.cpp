
#include <sti/utils/Configuration.h>

using STI::Utils::Configuration;



Configuration::Configuration()
{
}

Configuration::Configuration(const std::map<std::string, std::string>& parameters)
{
	configData[""].section = "";
	configData[""].parameters = parameters;
}

Configuration::Configuration(const std::map<std::string, std::map<std::string, std::string>>& config)
{
	for (auto& sec : config) {
		configData[sec.first].section = sec.first;
		configData[sec.first].parameters = sec.second;
	}
}

std::vector<std::string> Configuration::getSectionNames() const
{
	std::vector<std::string> sections;

	for (auto& data : configData) {
		sections.push_back(data.first);
	}
	return sections;
}

std::map<std::string, std::string> Configuration::getParameters(const std::string& section) const
{
	auto sectionData = configData.find(section);

	if (sectionData != configData.end()) {
		return sectionData->second.parameters;
	}

	std::map<std::string, std::string> parameters;
	return parameters;
}

bool Configuration::includes(const std::string& name) const
{
	return includes("", name);
}

bool Configuration::includes(const std::string& section, const std::string& name) const
{
	std::string value;
	return getStringValue(section, name, value);
}

Configuration& Configuration::set(const std::string& section, const std::string& name, const std::string& value)
{
	return setStringValue(section, name, value);
}

Configuration& Configuration::setStringValue(const std::string& section, const std::string& name, const std::string& value)
{
	configData[section].section = section;
	configData[section].parameters[STI::Utils::trim(name)] = STI::Utils::trim(value);
	return (*this);
}


bool Configuration::getStringValue(const std::string& section, const std::string& name, std::string &value) const
{
	auto sectionData = configData.find(section);
	
	if (sectionData == configData.end()) {
		return false;
	}

	auto param = sectionData->second.parameters.find(name);

	if (param == sectionData->second.parameters.end()) {
		return false;
	}

	value = param->second;
	return true;
}

Configuration& Configuration::append(const Configuration& config)
{
    auto newSections = config.getSectionNames();

    for (auto& section : newSections) {
        //check if the section also exists in this instance
        auto it = configData.find(section);
     
        if (it != configData.end()) {
            //existing section
            configData[section].merge(config.configData.at(section));
        }
        else {
            //new section
            configData[section] = config.configData.at(section);
        }
    }
	return (*this);
}

Configuration& Configuration::append(const std::map<std::string, std::map<std::string, std::string>>& config)
{
	Configuration newConfig(config);
	return append(newConfig);
}

Configuration& Configuration::operator+(const Configuration& config)
{
	return append(config);
}

Configuration& Configuration::operator+(const std::map<std::string, std::map<std::string, std::string>>& config)
{
	return append(config);
}