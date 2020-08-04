
#include "TDeviceCollection_i.h"
#include "RemoteDevice.h"
#include "NetworkConvert.h"
#include "NetworkDeviceWrapper.h"

using STI::Network::NetworkDeviceWrapper;
using STI::Network::RemoteDevice;
using STI::TNetwork::TDeviceCollection_i;
using ::STI::TNetwork::TDeviceID;
using STI::Device::DeviceID;
using STI::Network::convert;


TDeviceCollection_i::TDeviceCollection_i(const std::shared_ptr<STI::Device::DeviceCollector>& collector)
{
	collector->getCollection(deviceCollection);
}

::CORBA::Boolean TDeviceCollection_i::add(const TDeviceID& deviceID, ::STI::TNetwork::TDevice_ptr device)
{
	if (deviceCollection != 0 && device != 0 && !device->_is_nil()) {
		
		//wrap the received TDevice reference in RemoteDevice
		std::shared_ptr<RemoteDevice> remoteDevice = std::make_shared<RemoteDevice>(device);
		
		return (remoteDevice != 0 &&
			deviceCollection->add(convert<TDeviceID, DeviceID>(deviceID), remoteDevice)
			);
	}
	return false;
}

::CORBA::Boolean TDeviceCollection_i::remove(const TDeviceID& deviceID)
{
	if (deviceCollection != 0) {
		return deviceCollection->remove(convert<TDeviceID, DeviceID>(deviceID));
	}
	return false;
}

::CORBA::Boolean TDeviceCollection_i::contains(const TDeviceID& deviceID)
{
	if (deviceCollection != 0) {
		return deviceCollection->contains(convert<TDeviceID, DeviceID>(deviceID));
	}
	return false;
}

::CORBA::Boolean TDeviceCollection_i::get(const TDeviceID& deviceID, ::STI::TNetwork::TDevice_out device)
{
	bool success = false;
	std::shared_ptr<STI::Device::Device> localDevice;

	if (deviceCollection != 0) {
		//get local device reference from Collection
		success = deviceCollection->get(convert<TDeviceID, DeviceID>(deviceID), localDevice)
			&& localDevice != 0;
	}

	if (!success) {
		return false;
	}

	STI::TNetwork::TDevice_ptr tDevice;
	success = NetworkDeviceWrapper::getTDeviceReference(localDevice, tDevice);

	if (success) {
		
		STI::TNetwork::TDevice_var tDeviceVar(tDevice);		//managed

		device = tDeviceVar.out();
	}

	return success;
}

void TDeviceCollection_i::getIDs(::STI::TNetwork::TDeviceIDSeq_out deviceIDseq)
{
	std::set<DeviceID> ids;
	if (deviceCollection != 0) {
		deviceCollection->getIDs(ids);

		STI::TNetwork::TDeviceIDSeq_var tDeviceIDseq_var(new STI::TNetwork::TDeviceIDSeq);

		convert<DeviceID, STI::TNetwork::TDeviceID>(ids,
			(_CORBA_Unbounded_Sequence<STI::TNetwork::TDeviceID>&) tDeviceIDseq_var);
	}
}

void TDeviceCollection_i::cleanup()
{
	if (deviceCollection != 0) {
		return deviceCollection->cleanup();
	}
}

void TDeviceCollection_i::clear()
{
	if (deviceCollection != 0) {
		return deviceCollection->clear();
	}
}
