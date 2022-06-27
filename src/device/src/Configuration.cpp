
#include <sti/utils/Configuration.h>

using STI::Utils::Configuration;



Configuration::Configuration()
{
}

std::vector<std::string> Configuration::getSectionNames() const
{
	std::vector<std::string> sections;

	for (auto& data : configData) {
		sections.push_back(data.first);
	}
	return sections;
}

Configuration& Configuration::setParameter(const std::string& section, const std::string& name, const std::string& value)
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

Configuration& Configuration::operator+(const Configuration& config)
{
	append(config);
	return (*this);
}
