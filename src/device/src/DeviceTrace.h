#ifndef STI_DEVICE_DEVICETRACE_H
#define STI_DEVICE_DEVICETRACE_H

#include "DeviceID.h"

#include <vector>
#include <algorithm>

namespace STI
{
namespace Device
{

//Identical to HubTrace
//Should be made a template class....
class DeviceTrace
{
public:

	DeviceTrace() {}

	DeviceTrace(const DeviceID& first) { addID(first); }
	DeviceTrace(const DeviceTrace& src) { ids = src.ids; }

	void addID(const DeviceID& id) { ids.push_back(id); }
	bool includesID(const DeviceID& id) const { return std::find(ids.begin(), ids.end(), id) != ids.end(); }
	const DeviceID first() const 
	{	
		if (!ids.empty()) {
			return ids.front(); 			
		}

		DeviceID id;	//empty
		return id;
	}

	const DeviceID last() const 
	{	
		if (!ids.empty()) {
			return ids.back(); 			
		}

		DeviceID id;	//empty
		return id;
	}
	
	unsigned size() const { return static_cast<unsigned>(ids.size()); }

	const std::vector<DeviceID>& getIDs() const { return ids; }

private:
	std::vector<DeviceID> ids;
};

} //Device
} //STI


#endif

