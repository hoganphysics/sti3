#ifndef STI_DEVICE_LOCALSERVER_H
#define STI_DEVICE_LOCALSERVER_H

#include <sti/LocalDevice.h>
#include <sti/device/DeviceID.h>


namespace STI
{

namespace Device
{

class DeviceMessageReceiver;
class LocalDeviceMessageDispatcher;
class LocalChannelManager;
class LocalChannel;



class ServerCollectionPolicy : public STI::Utils::LocalCollection<DeviceID, Device>::LocalCollectionPolicy
{
public:
	ServerCollectionPolicy(LocalServer* server) : server(server) {}
	
	bool include(const STI::Device::DeviceID& key) const { return server->isTargetServer(key); }
	bool replace(const STI::Device::DeviceID& oldKey, const STI::Device::DeviceID& newKey) const { return (oldKey == newKey); }

private:
	LocalServer* server;
};



class LocalServer : public LocalDevice
{
public:
	
	LocalServer(const std::string& name, const std::string& address, unsigned short module,
		const std::string& targetServer)
        : LocalDevice(name, address, module, targetServer)
        {

        }
	virtual ~LocalServer() {}

private:

    friend ServerCollectionPolicy;
    bool isTargetServer(const DeviceID& id)
    {
        return id.getTargetServerID() == getID().getID();
    }

};

} //Device
} //STI

#endif
