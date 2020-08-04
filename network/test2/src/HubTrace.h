#ifndef STI_NETWORK_HUBTRACE_H
#define STI_NETWORK_HUBTRACE_H

#include "HubID.h"

#include <vector>

namespace STI
{
namespace Network
{


class HubTrace
{
public:

	HubTrace() {}

	HubTrace(const HubID& first) { addHubID(first); }
	HubTrace(const HubTrace& src) { ids = src.ids; }

	void addHubID(const HubID& id) { ids.push_back(id); }
	bool includesHubID(const HubID& id) const { return std::find(ids.begin(), ids.end(), id) != ids.end(); }
	const HubID& first() const { return ids.at(0); }
	unsigned size() const { return static_cast<unsigned>(ids.size()); }

	const std::vector<HubID>& getIDs() const { return ids; }

private:
	std::vector<HubID> ids;
};

} //Network
} //STI


#endif

