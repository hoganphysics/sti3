
#include "STIPyChannel.h"

using STI::Python::STIPyChannel;
using STI::Python::STIPyDevice;

STIPyChannel::STIPyChannel(const std::string& name)
: abstract_(true), abstractName_(name)
{
}

STIPyChannel::STIPyChannel(const std::shared_ptr<STIPyDevice>& device, const std::string& name)
: device_(device), abstract_(true), abstractName_(name)
{
}


STIPyChannel::STIPyChannel(const std::shared_ptr<STIPyDevice>& device, unsigned channel)
: device_(device), abstract_(false), channel_(channel)
{
}


bool STIPyChannel::isAbstract() const
{
    return abstract_;
}

const std::string& STIPyChannel::abstractName() const
{
    return abstractName_;
}

std::shared_ptr<STIPyDevice> STIPyChannel::device() const
{
    return device_;
}

unsigned STIPyChannel::channel() const
{
    return channel_;
}

