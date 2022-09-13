#ifndef STI_NETWORK_REMOTECHANNELMANAGER_H
#define STI_NETWORK_REMOTECHANNELMANAGER_H

#include <sti/device/ChannelManager.h>
#include <sti/device/DeviceMessageListener.h>
#include "fwd/DeviceMessageListenerForwarder_fwd.h"
#include "deviceNet.h"
#include "TReferenceHolder.h"
#include <sti/device/DeviceID.h>
#include <sti/device/DeviceMessage.h>

#include <memory>
#include <mutex>
#include <map>


namespace STI
{
namespace Network
{

class RemoteChannel;

struct ChannelDataTuple
{
	std::string name;
	STI::Utils::MixedValue value;
};


class RemoteChannelManager : public STI::Device::ChannelManager,
                             public STI::TNetwork::TReferenceHolder<STI::TNetwork::TChannelManager>	//mixin
{
public:

	RemoteChannelManager(::STI::TNetwork::TChannelManager_ptr channelManager, 
                            const std::shared_ptr<STI::Device::DeviceMessageListenerForwarder>& forwarder,
                            const STI::Device::DeviceID& remoteID);
	~RemoteChannelManager();

    void getChannels(std::vector<std::shared_ptr<STI::Device::Channel>>& channels);
    bool getChannel(short channelNumber, std::shared_ptr<STI::Device::Channel>& channel);

    bool writeChannel(short channel, const STI::Utils::MixedValue& value);
    bool readChannel(short channel, const STI::Utils::MixedValue& value, STI::Utils::MixedValue& data);
    void stop();

    bool setChannelName(short channel, const std::string& name);
    bool ping() const;

	std::string getChannelName(short channel) const;
	STI::Utils::MixedValue getLastValue(short channel) const;

private:
	

    // struct ChannelDataTuple
    // {
    //     std::string name;
    //     STI::Utils::MixedValue value;
    // };
    // std::map<short, ChannelDataTuple> channelData;
    std::map<short, std::shared_ptr<ChannelDataTuple>> channelData;

    void setChannelData(const std::shared_ptr<RemoteChannel>& channel);

    STI::Utils::MixedValue _getLastValue(short channel) const;
    std::string _getChannelName(short channel) const;

    friend class ChannelUpdater;

    //ChannelUpdater
    class ChannelUpdater : public STI::Device::DeviceMessageListener<STI::Device::ChannelUpdateMessage>
    {
    public:

        ChannelUpdater(RemoteChannelManager* manager) : channelManager(manager) {}

        void handleMessage(const std::shared_ptr<STI::Device::ChannelUpdateMessage>& mess)
        {
            if (channelManager == 0) return;
            
            channelManager->handleMessage(mess);
        }

    private:

        RemoteChannelManager* channelManager;
    };

    void handleMessage(const std::shared_ptr<STI::Device::ChannelUpdateMessage>& mess);

    STI::Device::DeviceID remoteID;
    STI::Device::DeviceMessageListenerID listenerID;

    //::STI::TNetwork::TChannelManager_var tChannelManager;		//remote reference

    std::shared_ptr<STI::Device::DeviceMessageListenerForwarder> listenerForwarder;

    mutable std::mutex managerMutex;

};


} //Network
} //STI

#endif

