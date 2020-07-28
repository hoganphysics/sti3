#ifndef STI_DEVICE_DEVICEID_H
#define STI_DEVICE_DEVICEID_H

#include <string>
#include <memory>

namespace STI
{
namespace Device
{

class DeviceIDBase
{
public:

	DeviceIDBase(const std::string& name, const std::string& address, unsigned short module, 
		const std::string& targetServer)
		: name_l(name), address_l(address), module_l(module), _targetServerName(targetServer) 
	{ regenerateID(); }

	const std::string& getName() const { return name_l; }
	const std::string& getAddress() const { return address_l; }
	unsigned short getModule() const { return module_l; }
	const std::string& getID() const { return deviceID_l; }
	const std::string& getTargetServerName() const { return _targetServerName; }

	void setName(const std::string& name) { name_l = name; regenerateID();}
	void setAddress(const std::string& address) { address_l = address; regenerateID();}
	void setModule(unsigned short module) { module_l = module; regenerateID();}
//	void setID(const std::string& deviceID) { deviceID_l = deviceID; }


private:

	void regenerateID();

	std::string name_l;
	std::string address_l;
	unsigned short module_l;
	std::string deviceID_l;

	std::string _targetServerName;

};

// Name: The name of the device itself
// Address: The name (or IP address) of the the computer that runs this device
// Module: A number further distinguishing the devices on a given address (address:module should be unique)
// Server: The server that manages this device. The device connects to this server at startup.

class DeviceID
{
public:
//	DeviceID();
	DeviceID(const std::string& name, const std::string& address, unsigned short module, 
		const std::string& targetServer);
	
	bool operator<(const DeviceID& rhs) const { return getID().compare(rhs.getID()) < 0; }
	bool operator==(const DeviceID& rhs) const { return getID().compare(rhs.getID()) == 0; }
	bool operator!=(const DeviceID& rhs) const { return !((*this)==rhs); }
	
	const std::string& getName() const { return deviceIDBase->getName(); }
	const std::string& getAddress() const { return deviceIDBase->getAddress(); }
	unsigned short getModule() const { return deviceIDBase->getModule(); }
	const std::string& getID() const { return deviceIDBase->getID(); }
	const std::string& getTargetServerName() const { return deviceIDBase->getTargetServerName(); }

	
	static bool stringToDeviceID(const std::string& deviceIDin, DeviceID& deviceIDout);

	static std::string generateID(const std::string& name, const std::string& address, unsigned short module);
	static std::string generateContext(const DeviceID& deviceID);

private:

	void setName(const std::string& name) { deviceIDBase->setName(name); }
	void setAddress(const std::string& address) { deviceIDBase->setAddress(address); }
	void setModule(unsigned short module) { deviceIDBase->setModule(module); }
//	void setID(const std::string& deviceID) { deviceIDBase->setID(deviceID); }

	std::shared_ptr<DeviceIDBase> deviceIDBase;
};




}
}

#endif
