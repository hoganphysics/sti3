#ifndef STI_DEVICE_DEVICE_H
#define STI_DEVICE_DEVICE_H

#include <sti/network/Node.h>
#include <sti/device/DeviceID.h>
#include <sti/fwd/EventEngineScheduler_fwd.h>
#include <sti/utils/MixedValue.h>

#include <memory>

namespace STI
{
namespace Device
{

class DeviceMessageListenerForwarder;
class DeviceMessageDispatcher;
class ChannelManager;
class AttributeManager;
class PersistenceManager;
class ProfileManager;
class Device;
class Attribute;


class Device : public STI::Network::Node<DeviceID, Device>
{
public:

	virtual ~Device() {}

	virtual const DeviceID getID() const = 0;
	virtual void kill() = 0;
	
	virtual void getMessageDispatcher(std::shared_ptr<DeviceMessageDispatcher>& dispatcher) = 0;
	virtual bool getEngineScheduler(std::shared_ptr<STI::Engine::EventEngineScheduler>& scheduler) = 0;
	virtual void getChannelManager(std::shared_ptr<ChannelManager>& manager) = 0;
	virtual void getAttributeManager(std::shared_ptr<AttributeManager>& manager) = 0;
	virtual bool getPersistenceManager(std::shared_ptr<PersistenceManager>& manager) = 0;
	virtual bool getProfileManager(std::shared_ptr<ProfileManager>& manager) = 0;

	virtual void attachMessageListenerForwarder(const std::shared_ptr<DeviceMessageListenerForwarder>& forwarder) = 0;	//or localDevice?

	virtual bool addto(const STI::Network::HubID& target) { return true; }

	//convenience functions
	virtual bool write(short channel, const STI::Utils::MixedValue& value) = 0;
	virtual bool read(short channel, const STI::Utils::MixedValue& value, STI::Utils::MixedValue& data) = 0;
	virtual void stopRW() = 0;
	virtual std::string getAttribute(const std::string& key) = 0;
	virtual bool setAttribute(const std::string& key, const std::string& value) = 0;
	virtual bool getAttribute(const std::string& key, std::shared_ptr<Attribute>& attribute) = 0;

};

} //Device
} //STI

#endif
