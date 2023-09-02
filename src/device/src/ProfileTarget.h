#ifndef STI_DEVICE_PROFILETARGET_H
#define STI_DEVICE_PROFILETARGET_H

#include <sti/device/Profile.h>

#include <string>
#include <map>


namespace STI
{
namespace Device
{

	
class ProfileTarget
{
public:

    virtual bool loadProfile(const std::shared_ptr<Profile>& profile) = 0;
    virtual bool saveProfile(const std::shared_ptr<Profile>& profile) = 0;
};

} //Device
} //STI

#endif
