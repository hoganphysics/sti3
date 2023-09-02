
#ifndef STI_PYTHON_PROFILEPY_H
#define STI_PYTHON_PROFILEPY_H

#include <sti/device/Profile.h>

#include "MixedValuePy.h"

#include <memory>

namespace STI
{
namespace Python
{


class ProfilePy
{
public:

    ProfilePy();
    ProfilePy(const std::string& name);
    ProfilePy(const STI::Device::Profile& profile);

    std::shared_ptr<STI::Device::Profile> toProfile();

    std::string name;
    STI::Device::ProfileType type;
    std::map<std::string, std::string> attributeData;
    std::map<short, STI::Python::MixedValuePy> channelData;

};


} //Python
} //STI

#endif

