#ifndef STI_DEVICE_ATTRIBUTEREFRESHLISTENER_H
#define STI_DEVICE_ATTRIBUTEREFRESHLISTENER_H


#include <string>


namespace STI
{
namespace Device
{


class AttributeRefreshListener
{
public:
    virtual void handleAttributeRefreshEvent(const std::string& key, const std::string& value) = 0;
};


} //Device
} //STI

#endif
