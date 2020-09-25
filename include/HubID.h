#ifndef STI_NETWORK_HUBID_H
#define STI_NETWORK_HUBID_H

#include "utils.h"

#include <string>
#include <sstream>
#include <functional>

namespace STI
{
namespace Network
{

class HubID
{
public:
	HubID() {}

	HubID(std::string name, std::string address, unsigned short module) : name(name), address(address), module(module) {}

	bool operator<(const HubID& rhs) const { return id().compare(rhs.id()) < 0; }
	bool operator==(const HubID& rhs) const { return id().compare(rhs.id()) == 0; }
	bool operator!=(const HubID& rhs) const { return !((*this) == rhs); }


	std::string name;
	std::string address;
	unsigned short module;

	std::string id() const
	{
		std::stringstream hubid;

		auto clean = std::bind(STI::Utils::replaceChars, std::placeholders::_1, "./", "_");// STI::Utils::replaceChars(address, "./", "_")

		hubid << clean(address) << "/" << module << "/" << clean(name);
		return hubid.str();

//		return name + address;
	}

	static bool stringToHubID(const std::string& id, HubID& hubID)
	{
		std::vector<std::string> tokens;
		STI::Utils::splitString(id, "/", tokens);

		unsigned short module;

		if (tokens.size() == 3 && STI::Utils::stringToValue(tokens.at(1), module)) {
			hubID.address = tokens.at(0);
			hubID.module = module;
			hubID.name = tokens.at(2);
			return true;
		}
		return false;
	}
};

} //Network
} //STI


#endif

