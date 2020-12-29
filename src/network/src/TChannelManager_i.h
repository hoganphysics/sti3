#ifndef STI_TNETWORK_TCHANNELMANAGER_I_H
#define STI_TNETWORK_TCHANNELMANAGER_I_H

#include "fwd/ChannelManager_fwd.h"

#include "Device.h"
#include "deviceNet.h"

#include <memory>

namespace STI
{
namespace TNetwork
{


class TChannelManager_i : public POA_STI::TNetwork::TChannelManager
{
public:

	TChannelManager_i(const std::shared_ptr<STI::Device::Device>& device);
	~TChannelManager_i();

    TChannelSeq* getChannels();
    ::CORBA::Boolean getChannel(::CORBA::Short channelNumber, ::STI::TNetwork::TChannel_out channel);
    ::CORBA::Boolean writeChannel(::CORBA::Short channel, const ::STI::TNetwork::TMixedValue& value);
    ::CORBA::Boolean readChannel(::CORBA::Short channel, const ::STI::TNetwork::TMixedValue& value, ::STI::TNetwork::TMixedValue_out data);


private:

    std::shared_ptr<STI::Device::ChannelManager> channelManager;

};


} //TNetwork
} //STI

#endif

