#ifndef STI_TNETWORK_TCHANNELMANAGER_I_H
#define STI_TNETWORK_TCHANNELMANAGER_I_H

#include "fwd/ChannelManager_fwd.h"

#include <sti/device/Device.h>
#include "generated/deviceNet.h"

#include <memory>

namespace STI
{
namespace TNetwork
{


class TChannelManager_i : public POA_STI::TNetwork::TChannelManager,
                          public PortableServer::RefCountServantBase
{
public:

	TChannelManager_i(const std::shared_ptr<STI::Device::Device>& device);
	~TChannelManager_i();

    void getChannels(::STI::TNetwork::TChannelSeq_out channels);
    ::CORBA::Boolean getChannel(::CORBA::Short channelNumber, ::STI::TNetwork::TChannel_out channel);
    ::CORBA::Boolean setChannelName(::CORBA::Short channelNumber, const char* name);
    ::CORBA::Boolean writeChannel(::CORBA::Short channel, const ::STI::TNetwork::TMixedValue& value);
    ::CORBA::Boolean readChannel(::CORBA::Short channel, const ::STI::TNetwork::TMixedValue& value, ::STI::TNetwork::TMixedValue_out data);
    void stop();
    ::CORBA::Boolean ping();

private:

    std::shared_ptr<STI::Device::ChannelManager> channelManager;

};


} //TNetwork
} //STI

#endif

