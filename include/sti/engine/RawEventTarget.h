#ifndef STI_ENGINE_RAWEVENTTARGET_H
#define STI_ENGINE_RAWEVENTTARGET_H


#include <sti/device/DeviceID.h>

#include <sti/engine/RawEventTargetChannel.h>
#include <sti/engine/RawEventTargetDevice.h>

#include <string>


namespace STI
{
namespace Engine
{

class RawEventTargetChannel;
class RawEventTargetDevice;


class RawEventTarget
{
public:

    RawEventTarget();

    RawEventTarget(const RawEventTargetDevice& dev, const RawEventTargetChannel& ch);

    RawEventTarget(const STI::Device::DeviceID& targetDevice, unsigned short channel);

    RawEventTarget(const STI::Device::DeviceID& targetDevice, const std::string& channelName);
    RawEventTarget(const std::string& deviceName, unsigned short channel);
    RawEventTarget(const std::string& deviceName, const std::string& channelName);

    RawEventTarget(const std::string& channelName);

    bool isAbstract() const;

    RawEventTargetDevice& getDevice();
    const RawEventTargetDevice& device() const;

    RawEventTargetChannel& getChannel();
    const RawEventTargetChannel& channel() const;

    bool operator<(const RawEventTarget& rhs) const;
	bool operator==(const RawEventTarget& rhs) const;
	bool operator!=(const RawEventTarget& rhs) const;

    template<class Archive>
    void serialize(Archive& archive);

private:

    RawEventTargetDevice _device;
    RawEventTargetChannel _channel;
};


} //Engine
} //STI

#endif
