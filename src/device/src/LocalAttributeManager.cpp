
#include "LocalAttributeManager.h"
#include "LocalAttribute.h"

#include "DeviceID.h"
#include "DeviceMessageDispatcher.h"
#include "DeviceMessage.h"

#include <set>

using STI::Device::LocalAttributeManager;
using STI::Device::Attribute;
using STI::Device::LocalAttribute;
using STI::Device::Attribute;
using STI::Device::DeviceID;
using STI::Device::DeviceMessageDispatcher;


LocalAttributeManager::LocalAttributeManager(const DeviceID& localID, const std::shared_ptr<DeviceMessageDispatcher>& dispatcher)
 : localID(localID), messageGrouper(dispatcher)
{
    messageGrouper.setWarmup(100);   //ms
    messageGrouper.setCooldown(500); //ms

    messageGrouper.start();
}

LocalAttributeManager::~LocalAttributeManager()
{
}


std::string LocalAttributeManager::getValue(const std::string& key)
{
    std::shared_ptr<Attribute> attribute;

    if (getAttribute(key, attribute) && attribute != 0) {
        return attribute->getValue();
    }
    return "";
}


bool LocalAttributeManager::getAttribute(const std::string& key, std::shared_ptr<Attribute>& attribute)
{
    std::shared_ptr<LocalAttribute> localAttribute;

    if (attributeMap.get(key, localAttribute)) {
        attribute = localAttribute;
        return (attribute != 0);
    }
    return false;
}


void LocalAttributeManager::getAttributes(std::vector<std::shared_ptr<Attribute>>& attributes)
{
    attributes.clear();

    std::set<std::string> keys;
    std::shared_ptr<Attribute> attribute;

    attributeMap.getKeys(keys);

    for (auto& key : keys) {
        if (getAttribute(key, attribute) && attribute != 0) {
            attributes.push_back(attribute);
        }
    }
}

void LocalAttributeManager::getAttributes(std::map<std::string, std::string>& attributes)
{
    attributes.clear();

    std::set<std::string> keys;
    attributeMap.getKeys(keys);

    std::shared_ptr<Attribute> attribute;
    
    for (auto& key : keys) {
        if (getAttribute(key, attribute) && attribute != 0) {
            attributes[attribute->getKey()] = attribute->getValue();
        }
    }
}

bool LocalAttributeManager::setValue(const std::string& key, const std::string& value)
{
    std::shared_ptr<Attribute> attribute;

    if (getAttribute(key, attribute) && attribute != 0) {
        return attribute->setValue(value);
    }
    return false;
}


bool LocalAttributeManager::addAttribute(const std::shared_ptr<LocalAttribute>& attribute)
{
    if (attribute != 0 && attributeMap.add(attribute->getKey(), attribute)) {
        attribute->addRefreshListener(this);
        return true;      
    }
    return false;
}


void LocalAttributeManager::handleAttributeRefreshEvent(const std::string& key, const std::string& value)
{
    auto message = std::make_shared<STI::Device::AttributeUpdateMessage>(localID, key, value);
    messageGrouper.addMessage(message);    
}

