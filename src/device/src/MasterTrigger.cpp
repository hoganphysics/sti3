
#include "MasterTrigger.h"

using STI::Engine::MasterTrigger;
using STI::Device::DeviceID;

MasterTrigger::MasterTrigger(const STI::Device::DeviceID& triggerDevice) 
: triggerDevice(triggerDevice), running(false)
{
    status.clear();
}

void MasterTrigger::arm(const std::vector<STI::Device::DeviceID>& ids)
{
	std::unique_lock<std::mutex> mtriggerLock(mtriggerMutex);

	status.clear();
	running = false;

	for (auto& id : ids) {
		status[id] = MasterTrigger::TriggerStatus::Arming;
	}
	//status[engine->localDeviceID] = MasterTrigger::TriggerStatus::Arming;
}

void MasterTrigger::arm(const DeviceID& id)
{
	std::unique_lock<std::mutex> mtriggerLock(mtriggerMutex);

	status[id] = MasterTrigger::TriggerStatus::Arming;
}

void MasterTrigger::waitForArm()
{
	//Wait for all devices to be in WaitingForTrigger state; device callback to this when then are ready
	std::unique_lock<std::mutex> mtriggerLock(mtriggerMutex);
	running = true;

	while (running && !_allStatusMatch(MasterTrigger::TriggerStatus::Waiting)) {
		mtriggerCondition.wait(mtriggerLock);
	}
}

bool MasterTrigger::allStatusMatch(const MasterTrigger::TriggerStatus& target)
{
	std::unique_lock<std::mutex> mtriggerLock(mtriggerMutex);
	return _allStatusMatch(target);
}

bool MasterTrigger::_allStatusMatch(const MasterTrigger::TriggerStatus& target)
{
	bool success = true;

	for (auto& s : status) {
		if (s.second != target) {
			success = false;
			break;
		}
	}
	return success;
}

void MasterTrigger::stop()
{
	std::unique_lock<std::mutex> mtriggerLock(mtriggerMutex);
	
	running = false;
	mtriggerCondition.notify_all();

}

void MasterTrigger::ready(const STI::Device::DeviceID& id)
{
	std::unique_lock<std::mutex> mtriggerLock(mtriggerMutex);

	auto it = status.find(id);
	if (it != status.end()) {
		it->second = MasterTrigger::TriggerStatus::Waiting;
	}
	mtriggerCondition.notify_all();
}

void MasterTrigger::triggerFired(const STI::Device::DeviceID& id)
{
	std::unique_lock<std::mutex> mtriggerLock(mtriggerMutex);

	auto it = status.find(id);
	if (it != status.end()) {
		it->second = MasterTrigger::TriggerStatus::Triggered;
	}
	mtriggerCondition.notify_all();
}
