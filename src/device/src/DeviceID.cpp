#include "DeviceID.h"
#include "utils.h"

#include <sstream>
#include <memory>
#include <functional>


#include "CerealArchives.h"
#include <cereal/types/memory.hpp>
#include <cereal/types/string.hpp>


using STI::Device::DeviceID;
using STI::Device::DeviceIDBase;

DeviceID::DeviceID()
{
	deviceIDBase = std::make_shared<DeviceIDBase>("", "", 0, "");
}

DeviceID::DeviceID(const std::string& deviceIDin)
: DeviceID()
{
	stringToDeviceID(deviceIDin, *this);
}

DeviceID::DeviceID(const std::string& name, const std::string& address, unsigned short module)
: DeviceID(name, address, module, "")
{
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

	STI::Utils::splitString(deviceIDin, "/", idComponents);

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
{
	auto clean = std::bind(STI::Utils::replaceChars, std::placeholders::_1, "./", "_");

	std::stringstream id;
	id << clean(address) << "/" << module << "/" << clean(name);

	return id.str();
}

std::string DeviceID::generateContext(const DeviceID& deviceID)
{
	// context example: STI/Device/192_54_22_1/module_1/DigitalOut/
	std::stringstream context;

	//Example: 192_168_1_2/3/DigitalOut/
	context << STI::Utils::replaceChars(deviceID.getAddress(), "./", "_") << "/"
		<< deviceID.getModule() << "/"
		<< STI::Utils::replaceChars(deviceID.getName(), "./", "_") << "/";

	return context.str();
}

DeviceIDBase::DeviceIDBase()
: DeviceIDBase("", "", 0, "")
{
}

void DeviceIDBase::regenerateID() { deviceID_l = DeviceID::generateID(name_l, address_l, module_l); }

template<class Archive>
void DeviceIDBase::serialize(Archive& archive)
{
	archive( 
		cereal::make_nvp("name", name_l), 
		cereal::make_nvp("address", address_l), 
		cereal::make_nvp("module", module_l), 
		cereal::make_nvp("deviceID", deviceID_l), 
		cereal::make_nvp("targetServerID", _targetServerID)
		); 
}

template<class Archive>
void DeviceID::serialize(Archive& archive)
{
	archive( cereal::make_nvp("DeviceIDBase", deviceIDBase) );
}


template void DeviceIDBase::serialize<cereal::XMLOutputArchive>( cereal::XMLOutputArchive& );
template void DeviceIDBase::serialize<cereal::XMLInputArchive>( cereal::XMLInputArchive& );

template void DeviceID::serialize<cereal::XMLOutputArchive>( cereal::XMLOutputArchive& );
template void DeviceID::serialize<cereal::XMLInputArchive>( cereal::XMLInputArchive& );
