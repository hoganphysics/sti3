#ifndef STI_DEVICE_LOCALDEVICE_H
#define STI_DEVICE_LOCALDEVICE_H

#include "Node.h"
#include "DeviceID.h"

namespace STI
{
namespace Device
{


class TempPolicy : public STI::Utils::LocalCollection<STI::Device::DeviceID, STI::Device::Device>::LocalCollectionPolicy
{
	bool include(const STI::Device::DeviceID& key) const { return true; }
	bool replace(const STI::Device::DeviceID& oldKey, const STI::Device::DeviceID& newKey) const { return (oldKey == newKey); }
};

class LocalDevice : public STI::Device::Device
{
public:
	LocalDevice(const std::string& name, const std::string& address, unsigned short module,
		const std::string& targetServer) : id(name, address, module, targetServer)
	{
		std::shared_ptr<TempPolicy> policy = std::make_shared<TempPolicy>();;
		localCollection = std::make_shared<STI::Utils::LocalCollection<STI::Device::DeviceID, STI::Device::Device>>(policy);
	}
	~LocalDevice()
	{
		cout << "Destructor: " << id.getName() << endl;
	}
	STI::Device::DeviceID id;

	bool refresh() { return true; }

	void write(unsigned input)
	{
		cout << "writting: " << input << endl;
	}
	void getCollection(std::shared_ptr<STI::Device::DeviceCollection>& collection)
	{
		collection = localCollection;
	}
	std::shared_ptr<STI::Utils::LocalCollection<STI::Device::DeviceID, STI::Device::Device>> localCollection;
};


} //Device
} //STI

#endif
