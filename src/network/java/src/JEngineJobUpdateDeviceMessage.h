#ifndef STI_DEVICE_JENGINEJOBUPDATEDEVICEMESSAGE_H
#define STI_DEVICE_JENGINEJOBUPDATEDEVICEMESSAGE_H

#include "DeviceMessage.h"
#include "DeviceMessageListener.h"
#include "JEventEngineJob.h"

#include <memory>


namespace STI
{
namespace Device
{

class JEngineJobUpdateDeviceMessage;
class JEngineJobUpdateDeviceMessageListener;


class JEngineJobUpdateDeviceMessage : public EngineJobUpdateDeviceMessage
{
public:
	
	JEngineJobUpdateDeviceMessage(const STI::Device::DeviceTrace& trace)
    : EngineJobUpdateDeviceMessage(trace)
    {
    }
    virtual ~JEngineJobUpdateDeviceMessage() {}

    EngineJobUpdateTarget getTargetList()
    {
        return targetList;
    }

    std::shared_ptr<STI::Engine::JEventEngineJob> getJEngineJob()
    {
        return jEngineJob;
    }

private:
	
    EngineJobUpdateTarget targetList;
	std::shared_ptr<STI::Engine::JEventEngineJob> jEngineJob;

    friend class JEngineJobUpdateDeviceMessageListener;

    static std::shared_ptr<JEngineJobUpdateDeviceMessage> makeJMessage(const EngineJobUpdateDeviceMessage& mess)
    {
        // auto jMess = std::make_shared<Concrete_JEngineJobUpdateDeviceMessage>(mess);
        // return std::static_pointer_cast<JEngineJobUpdateDeviceMessage>(jMess);
        auto jMess = std::make_shared<JEngineJobUpdateDeviceMessage>(mess);
        return jMess;
    }

public:

    JEngineJobUpdateDeviceMessage(const EngineJobUpdateDeviceMessage& mess)
    : EngineJobUpdateDeviceMessage(mess.getDeviceTrace())
    {
        targetList = mess.targetList;
        jEngineJob = std::make_shared<STI::Engine::JEventEngineJob>(mess.engineJob);
    }

};

// struct Concrete_JEngineJobUpdateDeviceMessage : public JEngineJobUpdateDeviceMessage {
//     Concrete_JEngineJobUpdateDeviceMessage(const EngineJobUpdateDeviceMessage& mess)
//     : JEngineJobUpdateDeviceMessage(mess) {}
// };



class JEngineJobUpdateDeviceMessageListener : public DeviceMessageListener<EngineJobUpdateDeviceMessage>
{
public:

    virtual ~JEngineJobUpdateDeviceMessageListener() {}

    virtual void handleMessage(const std::shared_ptr<JEngineJobUpdateDeviceMessage>& mess) = 0;

private:

    void handleMessage(const std::shared_ptr<EngineJobUpdateDeviceMessage>& mess) {}

    void handleMessage2(const std::shared_ptr<EngineJobUpdateDeviceMessage>& mess) 
    {
        if (mess == 0 || mess->engineJob == 0) return;

        auto jMess = JEngineJobUpdateDeviceMessage::makeJMessage(*mess);

        JEngineJobUpdateDeviceMessageListener::handleMessage(jMess);
    }

};




} //Device
} //STI

#endif
