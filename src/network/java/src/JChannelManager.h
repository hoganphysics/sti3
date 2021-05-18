
#ifndef STI_DEVICE_JCHANNELMANAGER_H
#define STI_DEVICE_JCHANNELMANAGER_H

#include "ChannelManager.h"

#include <memory>


namespace STI
{
namespace Device
{

class Channel;
class ChannelManager;


//Java ChannelManager wrapper
class JChannelManager
{
public:
	
	JChannelManager(std::shared_ptr<STI::Device::ChannelManager>& manager);
	~JChannelManager();

    std::vector<std::shared_ptr<Channel>> getChannels();
    std::shared_ptr<Channel> getChannel(short channelNumber);

    bool writeChannel(short channel, const STI::Utils::MixedValue& value);
    STI::Utils::MixedValue readChannel(short channel, const STI::Utils::MixedValue& value);

private:

    std::shared_ptr<STI::Device::ChannelManager> localManager;

};

} //Device
} //STI

#endif

