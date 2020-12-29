#ifndef STI_NETWORK_REMOTECHANNELMANAGER_H
#define STI_NETWORK_REMOTECHANNELMANAGER_H

#include "ChannelManager.h"

#include "deviceNet.h"

#include <memory>

namespace STI
{
namespace Network
{

class LocalDevice;


class RemoteChannelManager : public STI::Device::ChannelManager
{
public:

	RemoteChannelManager(::STI::TNetwork::TChannelManager_ptr channelManager);
	~RemoteChannelManager() {}

    void getChannels(std::vector<std::shared_ptr<STI::Device::Channel>>& channels);
    bool getChannel(short channelNumber, std::shared_ptr<STI::Device::Channel>& channel);

    bool writeChannel(short channel, const STI::Utils::MixedValue& value);
    bool readChannel(short channel, const STI::Utils::MixedValue& value, STI::Utils::MixedValue& data);

private:
	
    ::STI::TNetwork::TChannelManager_var tChannelManager;		//remote reference

};


} //Network
} //STI

#endif

