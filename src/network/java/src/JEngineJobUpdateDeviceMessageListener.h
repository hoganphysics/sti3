#ifndef STI_DEVICE_JENGINEJOBUPDATEDEVICEMESSAGELISTENER_H
#define STI_DEVICE_JENGINEJOBUPDATEDEVICEMESSAGELISTENER_H

#include "JEngineJobUpdateDeviceMessage.h"
#include "DeviceMessageListener.h"


#include <memory>

#include <iostream>

namespace STI
{
namespace Device
{

class JEngineJobUpdateDeviceMessage;


class JEngineJobUpdateDeviceMessageListener : public DeviceMessageListener<EngineJobUpdateDeviceMessage>
{
public:

    JEngineJobUpdateDeviceMessageListener() : DeviceMessageListener<EngineJobUpdateDeviceMessage>() {}

    virtual ~JEngineJobUpdateDeviceMessageListener() {}

    virtual void handleJMessage(const std::shared_ptr<JEngineJobUpdateDeviceMessage>& mess) = 0;


// private:

    // void handleMessage(const std::shared_ptr<EngineJobUpdateDeviceMessage>& mess) {}

    void handleMessage(const std::shared_ptr<EngineJobUpdateDeviceMessage>& mess) 
    {
        // std::cout << "JEngineJobUpdateDeviceMessageListener::handleMessage " << std::endl;
        if (mess == 0 || mess->engineJob == 0) return;

        auto jMess = JEngineJobUpdateDeviceMessage::makeJMessage(*mess);

        handleJMessage(jMess);

        // std::cout << "Job handleMessage done" << std::endl;
    }

};




} //Device
} //STI

#endif
