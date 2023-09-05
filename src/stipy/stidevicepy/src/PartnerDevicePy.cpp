
#include "PartnerDevicePy.h"

using STI::Python::PartnerDevicePy;
using STI::Device::PartnerDevice;


PartnerDevicePy::PartnerDevicePy(const PartnerDevice& partner)
: partner(partner)
{
}

PartnerDevicePy::~PartnerDevicePy()
{
}

void PartnerDevicePy::addEvent(const STI::Engine::RawEvent& evt, const STI::Engine::RawEvent& referenceEvent)
{
    partner.addEvent(evt, referenceEvent);
}

void PartnerDevicePy::addEvent(double time, const STI::Engine::RawEventTargetChannel& channel, const pybind11::object& value, const STI::Engine::RawEvent& referenceEvent)
{
    MixedValuePy mValue(value);
    partner.addEvent(time, channel, mValue.getMixedValue(), referenceEvent);
}

void PartnerDevicePy::addEvent(double time, const STI::Engine::RawEventTargetChannel& channel, const pybind11::object& value, const STI::Engine::RawEventType& eventType, const STI::Engine::RawEvent& referenceEvent)
{
    MixedValuePy mValue(value);
    partner.addEvent(time, channel, mValue.getMixedValue(), eventType, referenceEvent);
}

