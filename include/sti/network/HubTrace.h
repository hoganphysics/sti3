#ifndef STI_NETWORK_HUBTRACE_H
#define STI_NETWORK_HUBTRACE_H

#include <sti/network/HubID.h>

#include <vector>
#include <algorithm>
#include <mutex>

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

	void addHubID(const HubID& id) 
	{ 
		std::lock_guard<std::mutex> lock(idsMutex);
		ids.push_back(id); 
	}

	bool includesHubID(const HubID& id) const 
	{
		std::lock_guard<std::mutex> lock(idsMutex);
		return std::find(ids.begin(), ids.end(), id) != ids.end();
	}

	const HubID first() const 
	{
		std::lock_guard<std::mutex> lock(idsMutex);

		if (!ids.empty()) {
			return ids.front(); 			
		}

		HubID id;	//empty
		return id;
	}

	unsigned size() const 
	{ 
		std::lock_guard<std::mutex> lock(idsMutex);
		return static_cast<unsigned>(ids.size()); 
	}

	void getIDs(std::vector<HubID>& hubIDs) const 
	{
		std::lock_guard<std::mutex> lock(idsMutex);
		hubIDs = ids;
	}

private:
	std::vector<HubID> ids;
	mutable std::mutex idsMutex;
};

} //Network
} //STI


#endif

