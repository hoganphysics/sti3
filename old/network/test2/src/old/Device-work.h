#ifndef STI_UTILS_HUB_H
#define STI_UTILS_HUB_H

#include <sti/utils/Collector.h>
#include <sti/utils/LocalCollection.h>

namespace STI
{
namespace Utils
{

class DeviceID
{
public:
	DeviceID(unsigned i) : id(i) {}
	unsigned id;
};

class Device : public Node<DeviceID, Device>
{
public:
	virtual void write(unsigned input) = 0;
};

class LocalDevice : public Device
{
public:
	void write(unsigned input)
	{
	}
	void getCollection(std::shared_ptr<STI::Utils::Collection<DeviceID, Device>>& collection)
	{
	}
};

class TRemoteDevice_ptr {};

class RemoteDevice : public Device
{
public:
	RemoteDevice(const DeviceID& id, TRemoteDevice_ptr tDevice)
	{
		//device_var = ::STI::Network::TRemoteDevice::_duplicate(tDevice)
	}

	void write(unsigned input)
	{
		//		device_var->write(input);
	}
	void getCollection(std::shared_ptr<STI::Utils::Collection<DeviceID, Device>>& collection)
	{
	}
};

class Hub
{
	void add()
	{

	}
	LocalCollection<ID, T>
};

class THub_i //servant
{
	void add(const TDeviceID& deviceID, TRemoteDevice_ptr device)
	{
		localHub->add()
	}
};

} //Utils
} //STI

#endif
