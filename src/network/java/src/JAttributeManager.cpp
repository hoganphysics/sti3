
#include "JAttributeManager.h"
#include "Attribute.h"

#include <iostream>

using STI::Device::JAttributeManager;
using STI::Device::Attribute;


JAttributeManager::JAttributeManager(std::shared_ptr<STI::Device::AttributeManager>& manager)
: localManager(manager)
{
}

JAttributeManager::~JAttributeManager()
{
}


std::string JAttributeManager::getValue(const std::string& key)
{
    if (localManager != 0) {
        return localManager->getValue(key);
    }
    return "";
}

bool JAttributeManager::setValue(const std::string& key, const std::string& value)
{
    if (localManager != 0) {
        return localManager->setValue(key, value);
    }
    return false;
}

std::shared_ptr<Attribute> JAttributeManager::getAttribute(const std::string& key)
{
    std::shared_ptr<Attribute> attribute;

    if (localManager != 0) {
        localManager->getAttribute(key, attribute);
    }
    return attribute;
}

std::vector<std::shared_ptr<Attribute>> JAttributeManager::getAttributes()
{
    std::vector<std::shared_ptr<Attribute>> attributes;

    if (localManager != 0) {
        localManager->getAttributes(attributes);
    }
    return attributes;
}

std::map<std::string, std::string> JAttributeManager::getAttributeTuples()
{
    std::map<std::string, std::string> attributeTuples;

    if (localManager != 0) {
        std::vector<std::shared_ptr<Attribute>> attributes;
        localManager->getAttributes(attributes);

        for (auto& attribute : attributes) {
            if (attribute != 0) {
                attributeTuples[attribute->getKey()] = attribute->getValue();
            }
        }

    }
    return attributeTuples;
}

