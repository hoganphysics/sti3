#ifndef STI_DEVICE_LOCALDEVICE_H
#define STI_DEVICE_LOCALDEVICE_H

#include "Device.h"
#include "DeviceID.h"
#include "DeviceCollection.h"
#include "LocalCollection.h"
#include "fwd/EventEngineScheduler_fwd.h"
#include "DeviceEventParser.h"

#include <string>

namespace STI
{

// //temp -- make fwd
// namespace Engine
// {
// class EventEngineScheduler;
// } // Engine

namespace Device
{

class DeviceEventReceiver;
class LocalDeviceEventDispatcher;


class DeviceCollectionPolicy : public STI::Utils::LocalCollection<DeviceID, Device>::LocalCollectionPolicy
{
	bool include(const STI::Device::DeviceID& key) const { return true; }
	bool replace(const STI::Device::DeviceID& oldKey, const STI::Device::DeviceID& newKey) const { return (oldKey == newKey); }
};


class LocalDevice : public Device, public STI::Engine::DeviceEventParser
{
public:
	
	LocalDevice(const std::string& name, const std::string& address, unsigned short module,
		const std::string& targetServer);
	virtual ~LocalDevice();

	DeviceID getID();

	bool refresh() { return true; }

	void write(unsigned input);	//temp

	void getCollection(std::shared_ptr<STI::Device::DeviceCollection>& collection);
	void getEventDispatcher(std::shared_ptr<DeviceEventDispatcher>& dispatcher);
	void getEventReceiver(std::shared_ptr<DeviceEventReceiver>& receiver);
	bool getEngineScheduler(std::shared_ptr<STI::Engine::EventEngineScheduler>& scheduler);

	DeviceID id;

	void parseEvents(const STI::Engine::RawEventMap& events, STI::Engine::SynchronousEventVector& synchedEvents) const {}
	void getEventTargets(std::set<DeviceID>& targetIDs)
	{
		targetIDs = eventTargets;
	}

private:

	class DeviceCollectionListener : public STI::Utils::LocalCollectionListenerAdapter<DeviceID>
	{
	public:
		DeviceCollectionListener(LocalDevice* localDevice) : localDevice(localDevice) {}

		void add(const DeviceID& id);
		void remove(const DeviceID& id);

		LocalDevice* localDevice;
	};
	//std::shared_ptr<DeviceCollectionListener> deviceCollectionListener;

	std::shared_ptr<STI::Utils::LocalCollection<DeviceID, Device>> localCollection;
	std::shared_ptr<LocalDeviceEventDispatcher> deviceEventDispatcher;
	std::shared_ptr<DeviceEventReceiver> deviceEventReceiver;
	std::shared_ptr<STI::Engine::EventEngineScheduler> eventEngineScheduler;

	std::set<DeviceID> eventTargets;
};

} //Device
} //STI

#endif
