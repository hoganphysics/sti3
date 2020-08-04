#ifndef STI_NETWORK_HUBID_H
#define STI_NETWORK_HUBID_H

#include <string>

namespace STI
{
namespace Network
{

class HubID
{
public:
	HubID() {}

	HubID(std::string name, std::string address) : name(name), address(address) {}

	bool operator<(const HubID& rhs) const { return id().compare(rhs.id()) < 0; }
	bool operator==(const HubID& rhs) const { return id().compare(rhs.id()) == 0; }
	bool operator!=(const HubID& rhs) const { return !((*this) == rhs); }


	std::string name;
	std::string address;

private:
	std::string id() const
	{
		return name + address;
	}
};

} //Network
} //STI


#endif

