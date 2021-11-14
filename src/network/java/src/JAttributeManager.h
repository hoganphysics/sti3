#ifndef STI_DEVICE_JATTRIBUTEMANAGER_H
#define STI_DEVICE_JATTRIBUTEMANAGER_H

#include "AttributeManager.h"

#include <memory>


namespace STI
{
namespace Device
{

class AttributeManager;


//Java AttributeManager wrapper
class JAttributeManager
{
public:
	
	JAttributeManager(std::shared_ptr<STI::Device::AttributeManager>& manager);
	~JAttributeManager();


    std::string getValue(const std::string& key);
    bool setValue(const std::string& key, const std::string& value);

    // bool getAttribute(const std::string& key, std::shared_ptr<Attribute>& attribute);
    // void getAttributes(std::vector<std::shared_ptr<Attribute>>& attributes);

private:

    std::shared_ptr<STI::Device::AttributeManager> localManager;

};

} //Device
} //STI

#endif
