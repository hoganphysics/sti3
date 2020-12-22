
#include "RemoteTriggerCallback.h"
#include "NetworkConvert.h"

using STI::Network::RemoteTriggerCallback;


RemoteTriggerCallback::RemoteTriggerCallback(::STI::TNetwork::TTriggerCallback_ptr trigger)
	: _tTrigger(STI::TNetwork::TTriggerCallback::_duplicate(trigger))
{
}

RemoteTriggerCallback::~RemoteTriggerCallback()
{
}

void RemoteTriggerCallback::ready(const STI::Device::DeviceID& id)
{
	try {
		_tTrigger->ready(convert<STI::Device::DeviceID, STI::TNetwork::TDeviceID>(id));
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
	try {
		_tTrigger->triggerFired(convert<STI::Device::DeviceID, STI::TNetwork::TDeviceID>(id));
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}
}
