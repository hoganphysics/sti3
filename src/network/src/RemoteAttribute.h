#ifndef STI_DEVICE_REMOTEATTRIBUTE_H
#define STI_DEVICE_REMOTEATTRIBUTE_H

#include <sti/device/Attribute.h>
#include <sti/utils/MetaData.h>

#include <string>
#include <vector>


namespace STI
{
namespace Network
{

class RemoteAttributeManager;

class RemoteAttribute : public STI::Device::Attribute
{
public:

    RemoteAttribute(const std::string& key, const std::string& value, 
                    const std::string& group, const std::vector<std::string>& allowedValues,
                    const STI::Utils::MixedValue& metaData);

    void attachManager(RemoteAttributeManager* manager);

	const std::string& getKey() const;
	const std::string& getValue() const;    //Calls RemoteAttributeManager to get updated value
    const std::string& getStoredValue() const;  //Last value stored in this instance
    const std::vector<std::string>& getAllowedValues() const;
    const std::string& getGroup() const;

    void refreshValue();

    bool setValue(const std::string& value);

	const STI::Utils::MixedValue& getMetaData() const;
	STI::Utils::MixedValue getMetaData(const std::string& key) const;


    // //For updates from push events
    // void updateValue(const std::string& value);

private:

    friend RemoteAttributeManager;
    std::string getCurrentValue() const;

    std::string key_;
    mutable std::string value_;     //value is stored in RemoteAttributeManager, so value_ is just a copy. Mutable so we can return (and store) a reference
    std::string group_;
    std::vector<std::string> allowedValues_;
    STI::Utils::MetaData metaData_;

    RemoteAttributeManager* remoteManager;
};


} //Network
} //STI

#endif
