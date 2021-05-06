#ifndef STI_DEVICE_CHANNELREFRESHLISTENER_H
#define STI_DEVICE_CHANNELREFRESHLISTENER_H

#include "fwd/MixedValue_fwd.h"

#include <string>


namespace STI
{
namespace Device
{


class ChannelRefreshListener
{
public:
    virtual void handleChannelRefreshEvent(short channelNumber, const STI::Utils::MixedValue& value) = 0;
    virtual void handleChannelNameRefreshEvent(short channelNumber, const std::string& name) = 0;
};


} //Device
} //STI

#endif
