#include "AttributeManagerPy.h"
#include <sti/device/LocalAttribute.h>

#include <pybind11/pybind11.h>

using STI::Python::AttributeManagerPy;


AttributeManagerPy::AttributeManagerPy(const std::shared_ptr<STI::Device::AttributeManager>& manager)
: attributeManager(manager)
{
}


std::string AttributeManagerPy::getValue(const std::string& key)
{
    if (attributeManager != 0) {
        return attributeManager->getValue(key);
    }
    //pybind11::key_error missingKey();
    //throw missingKey;

    return "";
}

bool AttributeManagerPy::setValue(const std::string& key, const std::string& value)
{
    if (attributeManager != 0) {
        return attributeManager->setValue(key, value);
    }
    return false;
}

bool AttributeManagerPy::refreshValue(const std::string& key)
{
    if (attributeManager != 0) {
        return attributeManager->refreshValue(key);
    }
    return false;
}

void AttributeManagerPy::refreshValues()
{
    if (attributeManager != 0) {
        attributeManager->refreshValues();
    }
}

std::shared_ptr<STI::Device::Attribute> AttributeManagerPy::getAttribute(const std::string& key)
{
    std::shared_ptr<STI::Device::Attribute> attribute;
    if (AttributeManagerPy::getAttribute(key, attribute)) {
        return attribute;
    }

    //pybind11::key_error missingKey();
    //throw missingKey;

    //not found
    attribute = std::make_shared<STI::Device::LocalAttribute>("1", "2");
    return attribute;
}

std::vector<std::shared_ptr<STI::Device::Attribute>> AttributeManagerPy::getAttributes()
{
    std::vector<std::shared_ptr<STI::Device::Attribute>> attributes;
    AttributeManagerPy::getAttributes(attributes);
    return attributes;
}

bool AttributeManagerPy::getAttribute(const std::string& key, std::shared_ptr<STI::Device::Attribute>& attribute)
{
    if (attributeManager != 0) {
        return attributeManager->getAttribute(key, attribute);
    }
    return false;
}

void AttributeManagerPy::getAttributes(std::vector<std::shared_ptr<STI::Device::Attribute>>& attributes)
{
    if (attributeManager != 0) {
        return attributeManager->getAttributes(attributes);
    }
}

void AttributeManagerPy::getAttributes(std::map<std::string, std::string>& attributes)
{
    if (attributeManager != 0) {
        return attributeManager->getAttributes(attributes);
    }
}
