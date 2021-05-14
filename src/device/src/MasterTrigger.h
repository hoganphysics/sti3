#ifndef STI_ENGINE_MASTERTRIGGER_H
#define STI_ENGINE_MASTERTRIGGER_H

#include "TriggerCallback.h"
#include "DeviceID.h"

#include <memory>
#include <mutex>
#include <condition_variable>
#include <map>
#include <vector>

namespace STI
{
namespace Engine
{


class MasterTrigger : public TriggerCallbackTarget
{
public:

    MasterTrigger(const STI::Device::DeviceID& triggerDevice);

    enum class TriggerStatus { Arming, Waiting, Triggered };

    void arm(const STI::Device::DeviceID& id);
    void arm(const std::vector<STI::Device::DeviceID>& ids);
    void waitForArm();
    void stop();
    bool allStatusMatch(const TriggerStatus& target);

    STI::Device::DeviceID& triggerID() { return triggerDevice; }

    //TriggerCallbackTarget
    void ready(const STI::Device::DeviceID& id);
    void triggerFired(const STI::Device::DeviceID& id);

private:

    std::map<STI::Device::DeviceID, TriggerStatus> status;

    bool _allStatusMatch(const TriggerStatus& target);

    STI::Device::DeviceID triggerDevice;

    bool running;

    mutable std::mutex mtriggerMutex;
    mutable std::condition_variable mtriggerCondition;
};



} //Engine
} //STI

#endif
