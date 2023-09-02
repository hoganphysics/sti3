#ifndef STI_DEVICE_LOCALCHANNELMANAGER_H
#define STI_DEVICE_LOCALCHANNELMANAGER_H

#include <sti/device/ChannelManager.h>
#include <sti/utils/SynchronizedMap.h>

#include "ChannelRefreshListener.h"
#include "DeviceMessageGrouper.h"
#include "PersistenceTarget.h"
#include "ProfileTarget.h"

#include <memory>
#include <atomic>
#include <functional>


namespace STI
{
namespace Device
{

class LocalDevice;
class DeviceMessageDispatcher;
class ChannelUpdateMessage;
class LocalChannel;


class LocalChannelManager : public ChannelManager,
                            public ChannelRefreshListener,
                            public PersistenceTarget,
                            public ProfileTarget
{
public:

	LocalChannelManager(LocalDevice* localDevice, const std::shared_ptr<DeviceMessageDispatcher>& dispatcher);
	~LocalChannelManager() {}

    void getChannels(std::vector<std::shared_ptr<Channel>>& channels);
    bool getChannel(short channelNumber, std::shared_ptr<Channel>& channel);

    bool writeChannel(short channel, const STI::Utils::MixedValue& value);
    bool readChannel(short channel, const STI::Utils::MixedValue& value, STI::Utils::MixedValue& data);
    void stop();

    void addChannel(const std::shared_ptr<LocalChannel>& channel);

    bool loadProfile(const std::shared_ptr<Profile>& profile);
    bool saveProfile(const std::shared_ptr<Profile>& profile);

private:

    //PersistenceTarget
    std::string getFilenameStem();
    std::string getHeader();
    void setPersistenceCallback(const std::function<void(void)>& refresher);
    void setPersistenceData(const std::shared_ptr<STI::Utils::Configuration>& data);
    bool save();
    void load();

    //ChannelRefreshListener
    void handleChannelRefreshEvent(short channelNumber, const STI::Utils::MixedValue& value);
    void handleChannelNameRefreshEvent(short channelNumber, const std::string& name);

    LocalDevice* localDevice;

    STI::Utils::SynchronizedMap<short, std::shared_ptr<Channel>> channelMap;

    STI::Device::DeviceMessageGrouper<ChannelUpdateMessage> messageGrouper;

    std::function<void(void)> persistenceRefresher;
    std::shared_ptr<STI::Utils::Configuration> persistenceData;
    std::atomic<bool> loading;
};


} //Device
} //STI

#endif

