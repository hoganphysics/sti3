#include "RemoteTriggerCallback.h"
#include "NetworkConvert.h"

using STI::Network::RemoteTriggerCallback;
using STI::TNetwork::TReferenceHolder;
using STI::TNetwork::TTriggerCallback;


RemoteTriggerCallback::RemoteTriggerCallback(::STI::TNetwork::TTriggerCallback_var trigger)
: TReferenceHolder<TTriggerCallback>(trigger)
{
}

RemoteTriggerCallback::~RemoteTriggerCallback()
{
	disable();
}

void RemoteTriggerCallback::ready(const STI::Device::DeviceID& id)
{
	std::unique_lock<std::mutex> cbLock(cbMutex);

	if (isDisabled()) return;

	try {
		getTRef()->ready(convert<STI::Device::DeviceID, STI::TNetwork::TDeviceID>(id));
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}
}

void RemoteTriggerCallback::triggerFired(const STI::Device::DeviceID& id)
{
	std::unique_lock<std::mutex> cbLock(cbMutex);

	if (isDisabled()) return;

	try {
		getTRef()->triggerFired(convert<STI::Device::DeviceID, STI::TNetwork::TDeviceID>(id));
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}
}
