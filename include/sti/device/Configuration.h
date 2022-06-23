
#ifndef STI_DEVICE_CONFIGURATION_H
#define STI_DEVICE_CONFIGURATION_H

#include <sti/utils/utils.h>

#include <map>
#include <string>
#include <vector>


namespace STI
{

namespace Device
{
/*
{
	"sec1": {(key, val), ...},
	...
}

*/

template<typename T>
class ConfigResult
{
public:

    // ConfigResult(const std::string& svalue)
    // {
    //     STI::Utils::stringToValue(svalue, value);
    // }

    // ConfigResult(const T& val)
    // {
    //     value = val;
    // }

    template<typename U>
    ConfigResult(const U& val)
    {
        //STI::Utils::stringToValue(STI::Utils::valueToString(val), value);
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

// template<>
// ConfigResult<std::string>::ConfigResult(const std::string& svalue)
// {
//     STI::Utils::stringToValue(svalue, value);
//     value = svalue;
// }


class Configuration
{
public:

	Configuration();
	virtual ~Configuration() {}

	std::vector<std::string> getSectionNames() const;

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

    template <class T>
	void setParameter(const std::string& name, const T& value)
    {
        setParameter("", name, value);
    }
    template <class T>
	void setParameter(const std::string& section, const std::string& name, const T& value)
    {
        setStringValue(section, name, STI::Utils::valueToString(value));
    }

	void setParameter(const std::string& section, const std::string& name, const std::string& value)
    {
        setStringValue(section, name, value);
    }

    void append(const Configuration& config);

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

	void setStringValue(const std::string& section, const std::string& name, const std::string& value);
	bool getStringValue(const std::string& section, const std::string& name, std::string& value) const;

	std::map<std::string, ConfigSection> configData;

};


} //Device
} //STI

#endif

