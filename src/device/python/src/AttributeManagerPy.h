
#ifndef STI_PYTHON_ATTRIBUTEMANAGERPY_H
#define STI_PYTHON_ATTRIBUTEMANAGERPY_H

#include "AttributeManager.h"
#include "Attribute.h"

#include <memory>
#include <vector>
#include <string>


namespace STI
{
namespace Python
{

class AttributeManagerPy : public STI::Device::AttributeManager
{
public:

    AttributeManagerPy(const std::shared_ptr<STI::Device::AttributeManager>& manager);
    
    std::string getValue(const std::string& key);
    bool setValue(const std::string& key, const std::string& value);

    std::shared_ptr<STI::Device::Attribute> getAttribute(const std::string& key);
    std::vector<std::shared_ptr<STI::Device::Attribute>> getAttributes();

private:
    
    bool getAttribute(const std::string& key, std::shared_ptr<STI::Device::Attribute>& attribute);
    void getAttributes(std::vector<std::shared_ptr<STI::Device::Attribute>>& attributes);

    std::shared_ptr<STI::Device::AttributeManager> attributeManager;

};


} //Python
} //STI

#endif

