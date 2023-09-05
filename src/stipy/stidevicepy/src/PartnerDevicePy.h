#ifndef STI_PYTHON_PARTNERDEVICEPY_H
#define STI_PYTHON_PARTNERDEVICEPY_H

#include "PartnerDevicePy.h"
#include "DevicePy.h"

#include <sti/device/PartnerDevice.h>
#include <sti/device/DeviceID.h>
#include <sti/engine/RawEvent.h>

#include <memory>
#include <pybind11/pybind11.h>

#include <iostream>

namespace STI
{
namespace Python
{

class PartnerDevicePy : public DevicePy
{
public:

    PartnerDevicePy(const STI::Device::PartnerDevice& partner);
    ~PartnerDevicePy();

	void addEvent(const STI::Engine::RawEvent& evt, const STI::Engine::RawEvent& referenceEvent);
	void addEvent(double time, const STI::Engine::RawEventTargetChannel& channel, 
                  const pybind11::object& value, const STI::Engine::RawEvent& referenceEvent);
	void addEvent(double time, const STI::Engine::RawEventTargetChannel& channel, const pybind11::object& value, 
                  const STI::Engine::RawEventType& eventType, const STI::Engine::RawEvent& referenceEvent);

private:

    STI::Device::PartnerDevice partner;
};


} //Python
} //STI

#endif

