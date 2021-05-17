
#include "JAttributeManager.h"

using STI::Device::JAttributeManager;

JAttributeManager::

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

