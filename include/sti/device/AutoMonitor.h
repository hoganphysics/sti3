#ifndef STI_DEVICE_AUTOMONITOR_H
#define STI_DEVICE_AUTOMONITOR_H

#include <sti/device/LocalMonitor.h>

#include <string>
#include <functional>


namespace STI
{
namespace Device
{

class AutoMonitor : public LocalMonitor
{
public:
	AutoMonitor(const std::string& id, double updateInterval_s, const std::function<STI::Utils::MixedValue(void)>& updater);

};


} //Device
} //STI

#endif
