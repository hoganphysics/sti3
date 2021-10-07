#ifndef STI_DEVICE_CHANNELMANAGER_H
#define STI_DEVICE_CHANNELMANAGER_H

#include "fwd/Channel_fwd.h"
#include "MixedValue.h"

#include <vector>
#include <memory>


namespace STI
{
namespace Device
{


class ChannelManager
{
public:

	virtual ~ChannelManager() {}

    virtual void getChannels(std::vector<std::shared_ptr<Channel>>& channels) = 0;
    virtual bool getChannel(short channelNumber, std::shared_ptr<Channel>& channel) = 0;

    virtual bool writeChannel(short channel, const STI::Utils::MixedValue& value) = 0;
    virtual bool readChannel(short channel, const STI::Utils::MixedValue& value, STI::Utils::MixedValue& data) = 0;
    virtual void stop() = 0;

};


} //Device
} //STI

#endif

