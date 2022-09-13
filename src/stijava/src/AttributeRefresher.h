#ifndef STI_DEVICE_ATTRIBUTEREFRESHER_H
#define STI_DEVICE_ATTRIBUTEREFRESHER_H

#include <string>

namespace STI
{
namespace Device
{

class AttributeRefresher
{
public:
	
	AttributeRefresher() {}
    virtual ~AttributeRefresher() {}

    virtual std::string refresh() { return ""; }

};

} //Device
} //STI

#endif
