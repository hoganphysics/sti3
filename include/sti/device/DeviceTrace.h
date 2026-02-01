#ifndef STI_DEVICE_DEVICETRACE_H
#define STI_DEVICE_DEVICETRACE_H

#include <sti/device/DeviceID.h>

#include <vector>
#include <algorithm>
#include <mutex>

namespace STI
{
namespace Device
{


class DeviceTrace
{
public:

	DeviceTrace() {}
	DeviceTrace(const DeviceID& first) { addID(first); }
	DeviceTrace(const DeviceTrace& src) 
	{
		std::lock_guard<std::mutex> lock(src.idsMutex);
		ids = src.ids; 
	}

	DeviceTrace& operator=(const DeviceTrace& src)
    {
        if (this == &src) return *this;
        // lock both mutexes without deadlock
        std::scoped_lock lock(idsMutex, src.idsMutex);
        ids = src.ids;
        return *this;
    }

	void addID(const DeviceID& id) 
	{
		std::lock_guard<std::mutex> lock(idsMutex);
		ids.push_back(id); 
	}

	bool includesID(const DeviceID& id) const
	{ 
		std::lock_guard<std::mutex> lock(idsMutex);
		return std::find(ids.begin(), ids.end(), id) != ids.end(); 
	}

	const DeviceID first() const 
	{	
		std::lock_guard<std::mutex> lock(idsMutex);

		if (!ids.empty()) {
			return ids.front(); 			
		}

		DeviceID id;	//empty
		return id;
	}

	const DeviceID last() const 
	{
		std::lock_guard<std::mutex> lock(idsMutex);
		if (!ids.empty()) {
			return ids.back(); 			
		}

		DeviceID id;	//empty
		return id;
	}
	
	unsigned size() const
	{ 
		std::lock_guard<std::mutex> lock(idsMutex);
		return static_cast<unsigned>(ids.size());
	}

	void getIDs(std::vector<DeviceID>& deviceIDs) const 
	{
		std::lock_guard<std::mutex> lock(idsMutex);
		deviceIDs = ids;
	}

	std::string print(const std::string& separator = " -> ") const
	{
		std::lock_guard<std::mutex> lock(idsMutex);
		std::string result;

		for (unsigned i = 0; i < ids.size(); ++i) {
			
			result += ids[i].getID();
			
			if (i < ids.size() - 1) {
				result += separator;				
			}
		}

		return result;
	}

private:

	std::vector<DeviceID> ids;

	mutable std::mutex idsMutex;

};

} //Device
} //STI


#endif

