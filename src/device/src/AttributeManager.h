#ifndef STI_DEVICE_ATTRIBUTEMANAGER_H
#define STI_DEVICE_ATTRIBUTEMANAGER_H


#include <vector>
#include <string>
#include <memory>


namespace STI
{
namespace Device
{

class Attribute;


class AttributeManager
{
public:

    virtual ~AttributeManager() {}
   
    virtual std::string getValue(const std::string& key) = 0;
    virtual bool setValue(const std::string& key, const std::string& value) = 0;

    virtual bool getAttribute(const std::string& key, std::shared_ptr<Attribute>& attribute) = 0;
    virtual void getAttributes(std::vector<std::shared_ptr<Attribute>>& attributes) = 0;

};


} //Device
} //STI

#endif
