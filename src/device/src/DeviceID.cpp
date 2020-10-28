#include "DeviceID.h"
#include "utils.h"

#include <sstream>
#include <memory>
#include <functional>

using namespace STI::Device;

DeviceID::DeviceID()
{
	deviceIDBase = std::make_shared<DeviceIDBase>("", "", 0, "");
}


DeviceID::DeviceID(const std::string& name, const std::string& address, unsigned short module, 
	const std::string& targetServer)
{ 
	deviceIDBase = std::make_shared<DeviceIDBase>(name, address, module, targetServer);
}

bool DeviceID::stringToDeviceID(const std::string& deviceIDin, DeviceID& deviceIDout)
{
	//deviceIDin format: name_address_module
	std::vector<std::string> idComponents;
	unsigned short module;
	bool success = false;

	//STI::Utils::splitString(deviceIDin, "_", idComponents);
	STI::Utils::splitString(deviceIDin, "/", idComponents);

	// if(idComponents.size() == 3) {
	// 	deviceIDout.setName(idComponents.at(0));
	// 	deviceIDout.setAddress(idComponents.at(1));
	// 	if(STI::Utils::stringToValue(idComponents.at(2), module)) {
	// 		deviceIDout.setModule(module);
	// 		success = true;
	// 	}
	// }
	if(idComponents.size() == 3) {
		deviceIDout.setName(idComponents.at(2));
		deviceIDout.setAddress(idComponents.at(0));
		if(STI::Utils::stringToValue(idComponents.at(1), module)) {
			deviceIDout.setModule(module);
			success = true;
		}
	}
	return success;
}

std::string DeviceID::generateID(const std::string& name, const std::string& address, unsigned short module)
//	static std::string generateID(const DeviceID& deviceID)
{
	auto clean = std::bind(STI::Utils::replaceChars, std::placeholders::_1, "./", "_");

	std::stringstream id;
	//id << name << "_" << address << "_" << module;
	id << clean(address) << "/" << module << "/" << clean(name);

	return id.str();
}

std::string DeviceID::generateContext(const DeviceID& deviceID)
{
	// context example: STI/Device/192_54_22_1/module_1/DigitalOut/
	std::stringstream context;
//	context << "STI/Device/"
//		<< STI::Utils::replaceChars(deviceID.getAddress(), "./", "_") << "/"
//		<< "module_" << deviceID.getModule() << "/"
//		<< STI::Utils::replaceChars(deviceID.getName(), "./", "_") << "/";

	//Example: 192_168_1_2/3/DigitalOut/
	context << STI::Utils::replaceChars(deviceID.getAddress(), "./", "_") << "/"
		<< deviceID.getModule() << "/"
		<< STI::Utils::replaceChars(deviceID.getName(), "./", "_") << "/";

	return context.str();
}

void DeviceIDBase::regenerateID() { deviceID_l = DeviceID::generateID(name_l, address_l, module_l); }
