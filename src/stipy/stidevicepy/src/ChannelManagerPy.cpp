
#include "ChannelManagerPy.h"
#include "MixedValuePy.h"
#include "MixedValue.h"

using STI::Python::ChannelManagerPy;
using STI::Device::ChannelManager;
using STI::Device::Channel;
using STI::Python::MixedValuePy;
using STI::Utils::MixedValue;

#include <pybind11/pybind11.h>
namespace py = pybind11;


ChannelManagerPy::ChannelManagerPy(const std::shared_ptr<ChannelManager>& manager)
: channelManager(manager)
{
}


std::shared_ptr<Channel> ChannelManagerPy::getChannelPy(short channelNumber)
{
    std::shared_ptr<Channel> channel;

    getChannel(channelNumber, channel);

    return channel;
}

std::vector<std::shared_ptr<Channel>> ChannelManagerPy::getChannelsPy()
{
    std::vector<std::shared_ptr<Channel>> channels;

    getChannels(channels);

    return channels;
}

bool ChannelManagerPy::writeChannelPy(short channel, const pybind11::object& value)
{
    MixedValuePy val;
    val.setValue_py(value);

    return writeChannel(channel, val);
}

py::object ChannelManagerPy::readChannelPy(short channel, const pybind11::object& value)
{
    MixedValuePy val;
    val.setValue_py(value);

    MixedValuePy data;

    bool success = readChannel(channel, val, data);

    if (success) {
        return data.getValue_py();
    }

    return py::none();
}

void ChannelManagerPy::stop()
{
    if (channelManager != 0) {
        channelManager->stop();
    }
}

void ChannelManagerPy::getChannels(std::vector<std::shared_ptr<Channel>>& channels)
{
    if (channelManager != 0) {
        channelManager->getChannels(channels);
    }
}

bool ChannelManagerPy::getChannel(short channelNumber, std::shared_ptr<Channel>& channel)
{
    if (channelManager != 0) {
        return channelManager->getChannel(channelNumber, channel);
    }
    return false;
}


bool ChannelManagerPy::writeChannel(short channel, const STI::Utils::MixedValue& value)
{
    if (channelManager != 0) {
        return channelManager->writeChannel(channel, value);
    }
    return false;
}

bool ChannelManagerPy::readChannel(short channel, const STI::Utils::MixedValue& value, STI::Utils::MixedValue& data)
{
    if (channelManager != 0) {
        return channelManager->readChannel(channel, value, data);
    }
    return false;
}

