#ifndef STI_DEVICE_JENGINEJOBUPDATEDEVICEMESSAGELISTENER_H
#define STI_DEVICE_JENGINEJOBUPDATEDEVICEMESSAGELISTENER_H

#include "JEngineJobUpdateDeviceMessage.h"
#include <sti/device/DeviceMessageListener.h>

#include <memory>


namespace STI
{
namespace Device
{

class JEngineJobUpdateDeviceMessage;


class JEngineJobUpdateDeviceMessageListener : public DeviceMessageListener<EngineJobUpdateDeviceMessage>
{
public:

    // JEngineJobUpdateDeviceMessageListener() : DeviceMessageListener<EngineJobUpdateDeviceMessage>() {}

    ~JEngineJobUpdateDeviceMessageListener() {}

    virtual void handleJMessage(const std::shared_ptr<JEngineJobUpdateDeviceMessage>& mess) = 0;


// private:

    void handleMessage(const std::shared_ptr<EngineJobUpdateDeviceMessage>& mess) 
    {
        if (mess == 0 || mess->getEngineJob() == 0) return;

        auto jMess = JEngineJobUpdateDeviceMessage::makeJMessage(*mess);

        handleJMessage(jMess);
    }

};




} //Device
} //STI

#endif
