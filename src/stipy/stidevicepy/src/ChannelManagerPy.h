
#ifndef STI_PYTHON_CHANNELMANAGERPY_H
#define STI_PYTHON_CHANNELMANAGERPY_H

#include <sti/device/ChannelManager.h>
#include "MixedValuePy.h"
#include <sti/device/Channel.h>

#include <memory>
#include <vector>

#include <pybind11/pybind11.h>

namespace STI
{
namespace Python
{

class ChannelManagerPy : public STI::Device::ChannelManager
{
public:

    ChannelManagerPy(const std::shared_ptr<STI::Device::ChannelManager>& manager);
    
    std::shared_ptr<STI::Device::Channel> getChannelPy(short channelNumber);
    std::vector<std::shared_ptr<STI::Device::Channel>> getChannelsPy();

    bool writeChannelPy(short channel, const pybind11::object& value);
    pybind11::object readChannelPy(short channel, const pybind11::object& value);

    // bool writeChannelPy(short channel, const STI::Python::MixedValuePy& value);
    // pybind11::object readChannelPy(short channel, const STI::Python::MixedValuePy& value);

    void stop();

private:
    
    void getChannels(std::vector<std::shared_ptr<STI::Device::Channel>>& channels);
    bool getChannel(short channelNumber, std::shared_ptr<STI::Device::Channel>& channel);

    bool writeChannel(short channel, const STI::Utils::MixedValue& value);
    bool readChannel(short channel, const STI::Utils::MixedValue& value, STI::Utils::MixedValue& data);

    std::shared_ptr<STI::Device::ChannelManager> channelManager;

};


} //Python
} //STI

#endif

