#ifndef STI_UTILS_HUB_H
#define STI_UTILS_HUB_H

#include <sti/utils/Collector.h>
#include <sti/utils/LocalCollection.h>

#include <memory>

namespace STI
{
namespace Utils
{

class AbstractNode {
public:
	virtual ~AbstractNode() = default;
	virtual std::shared_ptr<AbstractNode> clone() const = 0;
};

template<class ID, class T>
class Node : public STI::Utils::Collector<ID, T>, public AbstractNode
{
public:
	std::shared_ptr<AbstractNode> clone() const override {
	//	return std::make_shared<T>(static_cast<T const&>(*this));
		return make();
	}
	virtual std::shared_ptr<AbstractNode> make() const = 0;
};


class DeviceID
{
public:
	DeviceID(unsigned i) : id(i) {}
	unsigned id;
};
class TDeviceID : DeviceID {};

class Device : public Node<DeviceID, Device>
{
public:
	virtual void write(unsigned input) = 0;
};

typedef std::shared_ptr<Device> Device_ptr;

class LocalDevice : public Device
{
public:
	void write(unsigned input)
	{
		std::cout << "Local: " << input << std::endl;
	}
	void getCollection(std::shared_ptr<STI::Utils::Collection<DeviceID, Device>>& collection)
	{
	}

	std::shared_ptr<AbstractNode> make() const
	{
		return std::make_shared<LocalDevice>(static_cast<LocalDevice const&>(*this));
	}

};

class TRemoteDevice_ptr {};

class RemoteDevice : public Device
{
public:
	RemoteDevice(TRemoteDevice_ptr tDevice)
	{
		//device_var = ::STI::Network::TRemoteDevice::_duplicate(tDevice)
	}

	TRemoteDevice_ptr getTRemoteDevice()
	{
		return device_var;
	}

	void write(unsigned input)
	{
		std::cout << "Remote: " << input << std::endl;
		//		device_var->write(input);
	}
	void getCollection(std::shared_ptr<STI::Utils::Collection<DeviceID, Device>>& collection)
	{
	}

	std::shared_ptr<AbstractNode> make() const
	{
		return std::make_shared<RemoteDevice>(static_cast<RemoteDevice const&>(*this));
	}
};

typedef std::shared_ptr<RemoteDevice> RemoteDevice_ptr;

class TRemoteDevice_i //servant
{
public:
	TRemoteDevice_i(const Device_ptr& device) { _device = device; }

	Device_ptr _device;
};

class TRemoteHub_ptr {};
class RemoteHub
{
public:
	RemoteHub(TRemoteHub_ptr tHub)
	{
		//hub_var = ::STI::Network::TRemoteHub::_duplicate(tHub);
	}
	void add(const DeviceID& id, const TRemoteDevice_ptr& node)
	{
		//hub_var->add(id, node);	//node needs to be: TRemoteDevice_ptr (from _this())
	}
	void add(const DeviceID& id, const RemoteDevice_ptr& node)
	{
		//hub_var->add(id, node);
	}


};

class NetworkHub;

class NetworkHub	//Network hub
{
	void addLocal(const DeviceID& id, const Device_ptr& node)
	{
		std::shared_ptr<TRemoteDevice_i> dev_i = std::make_shared<TRemoteDevice_i>(node);	//servant for this local device
//		servantnodes.add(id, dev_i);		//store servant locally

		//conditionally add to all locally stored devices
		for (s : servantnodes) {
			s._device->add(id, node);
			//what about just: s.add(id, node);
		}

		//distributeAdd(node): (to all RemoteHubs)
//		for (h : hubs) {
//			h->add(id, dev_i->_this());		//_this() returns TRemoteDevice_ptr
//		}
	}

	//called by THub_i servant
	void addRemote(const DeviceID& id, const RemoteDevice_ptr& node)
	{
		//conditionally add to all locally stored devices
		for (s : servantnodes) {
			s._device->add(id, node);		//accepts Device_ptr, which is convertible from RemoteDevice_ptr
		}

		//distributeAdd(node): (to all RemoteHubs)
//		for (h : hubs) {
//			h->add(id, node);
//		}

	}

//	LocalCollection<DeviceID, TRemoteDevice_i> servantnodes;
//	LocalCollection<DeviceID, RemoteHub> hubs;
};

//	LocalCollection<DeviceID, Device> nodes;


//class THub_i //servant
//{
//	void add(const TDeviceID& id, TRemoteDevice_ptr device)
//	{
//		auto d = make_shared<RemoteDevice>(device);	//wrap remote reference
//		localHub->addRemote(id, d);
//	}
//};

} //Utils
} //STI

#endif
