#ifndef STI_DEVICE_JENGINEJOBUPDATEDEVICEMESSAGE_H
#define STI_DEVICE_JENGINEJOBUPDATEDEVICEMESSAGE_H

#include "DeviceMessage.h"
#include "DeviceMessageListener.h"
#include "JEventEngineJob.h"
#include "EventEngineJobList.h"

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

    STI::Engine::EventEngineJobList getTargetList()
    {
        return targetList;
    }

    std::shared_ptr<STI::Engine::JEventEngineJob> getJEngineJob()
    {
        return jEngineJob;
    }

private:
	
    STI::Engine::EventEngineJobList targetList;
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
        targetList = mess.getTargetList();
        jEngineJob = std::make_shared<STI::Engine::JEventEngineJob>(mess.getEngineJob());
    }

};

// struct Concrete_JEngineJobUpdateDeviceMessage : public JEngineJobUpdateDeviceMessage {
//     Concrete_JEngineJobUpdateDeviceMessage(const EngineJobUpdateDeviceMessage& mess)
//     : JEngineJobUpdateDeviceMessage(mess) {}
// };



} //Device
} //STI

#endif
