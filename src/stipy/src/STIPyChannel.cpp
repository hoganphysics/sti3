
#include "STIPyChannel.h"

#include <sstream>

using STI::Python::STIPyChannel;
using STI::Python::STIPyDevice;


STIPyChannel::STIPyChannel(const std::string& name)
: abstract_(true), abstractName_(name), hasDevice_(false)
{
}

STIPyChannel::STIPyChannel(const std::shared_ptr<STIPyDevice>& device, const std::string& name)
: device_(device), abstract_(true), abstractName_(name), hasDevice_(true)
{
}


STIPyChannel::STIPyChannel(const std::shared_ptr<STIPyDevice>& device, unsigned channel)
: device_(device), channel_(channel), hasDevice_(true), abstract_(false)
{
    if (device == 0) {
        abstract_ = true;
        hasDevice_ = false;
    }
    if(device != 0 && device->isAbstract()) {
        abstract_ = true;
    }
}


bool STIPyChannel::isAbstract() const
{
    return abstract_;
}

std::string STIPyChannel::abstractName() const
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

std::string STIPyChannel::print() const
{
    std::stringstream s;
    
    if (hasDevice_ && device_ != 0) {
        s << device()->print();
        s << ".";
    }

    s << "ch(";    
    if (isAbstract()) {
        s << abstractName();
    }
    else {
        s << channel();
    }
    s << ")";

    return s.str();
}

