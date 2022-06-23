#ifndef STI_DEVICE_ATTRIBUTE_H
#define STI_DEVICE_ATTRIBUTE_H

#include <sti/utils/MixedValue.h>

#include <string>
#include <vector>

namespace STI
{
namespace Device
{

class Attribute;


class Attribute
{
public:

	virtual ~Attribute() {}

	virtual const std::string& getKey() const = 0;
	virtual const std::string& getValue() const = 0;
    virtual const std::vector<std::string>& getAllowedValues() const = 0;
    virtual const std::string& getGroup() const = 0;

    virtual void refreshValue() = 0;

    virtual bool setValue(const std::string& value) = 0;

	//Metadata can be used for GUI layout, tooltips, units, etc.
	virtual const STI::Utils::MixedValue& getMetaData() const = 0;
	virtual STI::Utils::MixedValue getMetaData(const std::string& key) const = 0;

};


} //Device
} //STI

#endif
