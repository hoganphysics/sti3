#ifndef STI_DEVICE_ATTRIBUTESETTER_H
#define STI_DEVICE_ATTRIBUTESETTER_H

#include <string>

namespace STI
{
namespace Device
{

class AttributeSetter
{
public:
	
	AttributeSetter() {}
    virtual ~AttributeSetter() {}

    virtual bool set(const std::string& value) { return false; }

};

} //Device
} //STI

#endif
