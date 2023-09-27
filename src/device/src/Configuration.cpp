
#include <sti/utils/Configuration.h>

#include <sstream>

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

std::vector<std::string> Configuration::getParameterNames() const
{
	return getParameterNames("");
}

std::vector<std::string> Configuration::getParameterNames(const std::string& section) const
{
	std::vector<std::string> names;

	auto sectionData = getParameters(section);

	for (auto& data : sectionData) {
		names.push_back(data.first);
	}
	return names;	
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

bool Configuration::isList(const std::string& name) const
{
	return isList("", name);
}

bool Configuration::isList(const std::string& section, const std::string& name) const
{
	std::string value;

	if (getStringValue(section, name, value)) {
		return (value.front() == '[' && value.back() == ']');
	}
	return false;
}

std::vector<std::string> Configuration::getList(const std::string& name) const
{
	return getList("", name);
}

std::vector<std::string> Configuration::getList(const std::string& section, const std::string& name) const
{
	std::vector<std::string> listValue;
	std::string value;

	if (!getStringValue(section, name, value)) {		//key not found
		return listValue;
	}

	if (!isList(section, name)) {	//key found, but not a list
		listValue.push_back(value);
		return listValue;
	}

	//is a list; parse value

	std::size_t listStart, listEnd, delimiterPos;
	std::string nextEntry;

	listStart = value.find_first_of("[");
	listEnd = value.find_last_of("]");

	value = value.substr(listStart + 1, listEnd - listStart - 1);

	do {
		delimiterPos = value.find_first_of(",");
		listValue.push_back( STI::Utils::trim( value.substr(0, delimiterPos) ) );	//trim whitespace and add next value
		value = value.substr(
					((delimiterPos == std::string::npos) ? 0 : delimiterPos + 1),
					std::string::npos);
	} 
	while (delimiterPos != std::string::npos);

	return listValue;
}

Configuration& Configuration::addToList(const std::string& section, const std::string& name, const std::string& value)
{
	auto values = getList(section, name);
	values.push_back(value);

	std::stringstream s;

	//List format: [v1, v2, v3, ...]
	s << "[";

	bool isFirst = true;
	for (auto& v : values) {
		if (!isFirst) {
			s << ", ";
		}
		s << v;
		isFirst = false;
	}

	s << "]";
	
	set(section, name, s.str());	//overwrite old value

	return (*this);
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

bool Configuration::hasPrefix(const std::string& item, const std::string& prefix)
{
    if (item.compare(prefix) == 0) return true;
    
    auto found = item.find(prefix);
    if (found != std::string::npos && item.size() > prefix.size()) {

        return item.substr(prefix.size(), 1).compare(".") == 0;
    }
    return false;
}

std::string Configuration::chopPrefix(const std::string& item, const std::string& prefix)
{
	if (item.compare(prefix) == 0) return "";

	auto found = item.find(prefix);
	if (found == 0 && item.size() > prefix.size()) {

		return item.substr(prefix.size() + 1, std::string::npos);
	}
	return item;
}

Configuration Configuration::extract(const std::string& section) const
{
	//could support subsections
	//[Camera 1]
	//[Camera 1.STI]
	//[.Network]
	//config.extract("Camera 1.*")
	//config.extract(dev + ".*")

	Configuration config;

	auto it = configData.find(section);

    //find all sections names with the prefix 'section'
    while (it != configData.end()) {
        if (hasPrefix(it->first, section)) {
			config + Configuration({{chopPrefix(it->first, section), getParameters(it->first)}});
        }
        ++it;
    }

	// return Configuration({{section, getParameters(section)}});

	return config;
}

Configuration Configuration::extract(const std::vector<std::string>& sections) const
{
	Configuration config;

	for (auto& section : sections) {
		config + extract(section);
	}
	return config;
}



Configuration Configuration::filter(const std::string& section) const
{
	//could support subsections
	//[Camera 1]
	//[Camera 1.STI]
	//[.Network]
	//config.extract("Camera 1.*")
	//config.extract(dev + ".*")

	Configuration config;

	auto it = configData.find(section);

	//find all sections names with the prefix 'section'
	while (it != configData.end()) {
		if (hasPrefix(it->first, section)) {
			config + Configuration({ {it->first, getParameters(it->first)} });
		}
		++it;
	}

	// return Configuration({{section, getParameters(section)}});

	return config;
}

Configuration Configuration::filter(const std::vector<std::string>& sections) const
{
	Configuration config;

	for (auto& section : sections) {
		config + filter(section);
	}
	return config;
}

void Configuration::clear()
{
	configData.clear();
}