
#include "RemoteAttribute.h"
#include "RemoteAttributeManager.h"


using STI::Network::RemoteAttribute;
using STI::Network::RemoteAttributeManager;


RemoteAttribute::RemoteAttribute(const std::string& key, const std::string& value, 
                    const std::string& group, const std::vector<std::string>& allowedValues,
                    const STI::Utils::MixedValue& metaData)
: key_(key), value_(value), group_(group), allowedValues_(allowedValues), metaData_(metaData)
{
}

void RemoteAttribute::attachManager(RemoteAttributeManager* manager)
{
    remoteManager = manager;
}

const std::string& RemoteAttribute::getKey() const
{
    return key_;
}

const std::string& RemoteAttribute::getValue() const
{
    if (remoteManager != 0) {
        value_ = remoteManager->getUpdatedValue(key_);
    }
    return value_;
}

const std::string& RemoteAttribute::getStoredValue() const
{
    return value_;
}

const std::vector<std::string>& RemoteAttribute::getAllowedValues() const
{
    return allowedValues_;
}

const std::string& RemoteAttribute::getGroup() const
{
    return group_;
}

void RemoteAttribute::refreshValue()
{
    if (remoteManager != 0) {
        value_ = remoteManager->getValue(key_);
    }
}

bool RemoteAttribute::setValue(const std::string& value)
{
    if (remoteManager != 0) {
        return remoteManager->setValue(key_, value_);
    }
    return false;
}

// void RemoteAttribute::updateValue(const std::string& value)
// {
//     value_ = value;
// }

const STI::Utils::MixedValue& RemoteAttribute::getMetaData() const
{
    return metaData_.getMetaData();
}

STI::Utils::MixedValue RemoteAttribute::getMetaData(const std::string& key) const
{
    return metaData_.getMetaData(key);
}

