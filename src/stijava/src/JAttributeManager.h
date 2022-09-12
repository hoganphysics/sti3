#ifndef STI_DEVICE_JATTRIBUTEMANAGER_H
#define STI_DEVICE_JATTRIBUTEMANAGER_H

#include <sti/device/AttributeManager.h>

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
	
	JAttributeManager(const std::shared_ptr<STI::Device::AttributeManager>& manager);
	~JAttributeManager();

    std::string getValue(const std::string& key);
    bool setValue(const std::string& key, const std::string& value);

    std::shared_ptr<Attribute> getAttribute(const std::string& key);
    std::vector<std::shared_ptr<Attribute>> getAttributes();
    std::map<std::string, std::string> getAttributeMap();

private:

    std::shared_ptr<STI::Device::AttributeManager> localManager;

};

} //Device
} //STI

#endif
