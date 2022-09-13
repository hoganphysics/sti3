
#ifndef STI_UTILS_CONFIGURATION_H
#define STI_UTILS_CONFIGURATION_H

#include <sti/utils/utils.h>

#include <map>
#include <string>
#include <vector>


namespace STI
{

namespace Utils
{


template<typename T>
class ConfigResult
{
public:

    template<typename U>
    ConfigResult(const U& val)
    {
        setValue(val);
    }

    operator T() const
    {
        return get();
    }

    T get() const
    {
        return value;
    }
    
private:

    void setValue(const T& val)
    {
        value = val;
    }

    template<typename U> void setValue(const U& val)
    {
        STI::Utils::stringToValue(STI::Utils::valueToString(val), value);
    }

    T value;
};


class Configuration;

class Configuration
{
public:

	Configuration();
    Configuration(const std::map<std::string, std::string>& parameters);   //a single, unnamed section
    Configuration(const std::map<std::string, std::map<std::string, std::string>>& config); //full configuration data
	virtual ~Configuration() {}

	std::vector<std::string> getSectionNames() const;
    std::vector<std::string> getParameterNames() const;
    std::vector<std::string> getParameterNames(const std::string& section) const;

    std::map<std::string, std::string> getParameters(const std::string& section) const;

    bool includes(const std::string& name) const;
    bool includes(const std::string& section, const std::string& name) const;

	template <class T>
	bool getParameter(const std::string& name, T& value) const
	{
		return getParameter("", name, value);
	}

	template <class T>
	bool getParameter(const std::string& section, const std::string& name, T& value) const
	{
		std::string strValue;
		if (!getStringValue(section, name, strValue)) {
			return false;
		}

		return STI::Utils::stringToValue(strValue, value);
	}

    template<typename T>
    ConfigResult<T> get(const std::string& key, const T& defaultValue) const
    {
        return get<T>("", key, defaultValue);
    }

    template<typename T>
    ConfigResult<T> get(const std::string& section, const std::string& key, const T& defaultValue) const
    {
        std::string value;
        if (!getStringValue(section, key, value)) {

            ConfigResult<T> tResult(defaultValue);
            return tResult;
        }

        ConfigResult<T> result(value);
        return result;
    }

    //values of the form [...] can be interpreted as a list
    bool isList(const std::string& name) const;
    bool isList(const std::string& section, const std::string& name) const;
    std::vector<std::string> getList(const std::string& name) const;
    std::vector<std::string> getList(const std::string& section, const std::string& name) const;

    template <class T>
	Configuration& set(const std::string& name, const T& value)
    {
        return set("", name, value);
    }
    template <class T>
	Configuration& set(const std::string& section, const std::string& name, const T& value)
    {
        return setStringValue(section, name, STI::Utils::valueToString(value));
    }
	Configuration& set(const std::string& section, const std::string& name, const std::string& value);
    
    template <class T>
	Configuration& addToList(const std::string& name, const T& value)
    {
        return addToList("", name, value);
    }
    template <class T>
	Configuration& addToList(const std::string& section, const std::string& name, const T& value)
    {
        return addToList(section, name, STI::Utils::valueToString(value));
    }
    Configuration& addToList(const std::string& section, const std::string& name, const std::string& value);

    Configuration& append(const Configuration& config);
    Configuration& append(const std::map<std::string, std::map<std::string, std::string>>& config);

    Configuration& operator+(const Configuration& config);
    Configuration& operator+(const std::map<std::string, std::map<std::string, std::string>>& config);

private:

	struct ConfigSection
	{
		std::string section;
		std::map<std::string, std::string> parameters;

        void merge(const ConfigSection& configSection)
        {
            parameters.insert(configSection.parameters.begin(), configSection.parameters.end());
        }
	};

	Configuration& setStringValue(const std::string& section, const std::string& name, const std::string& value);
	bool getStringValue(const std::string& section, const std::string& name, std::string& value) const;

	std::map<std::string, ConfigSection> configData;

};


} //Utils
} //STI

#endif

