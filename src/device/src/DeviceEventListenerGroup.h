#ifndef STI_DEVICE_DEVICEEVENTLISTENERGROUP_H
#define STI_DEVICE_DEVICEEVENTLISTENERGROUP_H

#include "DeviceEvent.h"
#include "DeviceEventListener.h"
#include "SynchronizedMap.h"

#include <memory>

namespace STI
{
namespace Device
{

class AbstractEventListenerGroup
{
public:

	virtual ~AbstractEventListenerGroup() {}

	virtual unsigned size() = 0;
	virtual void handleEvent(const std::shared_ptr<DeviceEvent>& evt) = 0;

};

template<class Event>
class DeviceEventListenerGroup : public AbstractEventListenerGroup
{
public:

	unsigned size() { return static_cast<unsigned>(listeners.size()); }

	void addListener(const DeviceEventListenerID& id, const std::shared_ptr<DeviceEventListener<Event>>& listener)
	{
		listeners.add(id, listener);
	}

	template<typename T>
	void addListener(const DeviceEventListenerID& id, T listener) {}	//catch

	void removeListener(const DeviceEventListenerID& id)
	{
		listeners.remove(id);
	}

	bool contains(const DeviceEventListenerID& id) const
	{
		return listeners.contains(id);
	}

	void handleEvent(const std::shared_ptr<DeviceEvent>& evt)
	{
		std::shared_ptr<Event> convertedEvent;

		if (evt->getType() == Event::getEventClassType() && 
			DeviceEvent::convert<Event>(evt, convertedEvent)) 
		{
			handleEvent(convertedEvent);
		}
	}

private:

	void handleEvent(const std::shared_ptr<Event>& evt)
	{
		std::set<DeviceEventListenerID> ids;
		listeners.getKeys(ids);

		std::shared_ptr<DeviceEventListener<Event>> listener;

		for (auto& id : ids) {
			if (listeners.get(id, listener) && listener != 0) {

				listener->handleEvent(evt);
			}
		}
	}

	STI::Utils::SynchronizedMap<DeviceEventListenerID, std::shared_ptr<DeviceEventListener<Event>>> listeners;

};

} //Device
} //STI


#endif

