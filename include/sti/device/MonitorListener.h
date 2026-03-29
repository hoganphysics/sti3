#ifndef STI_DEVICE_MONITORLISTENER_H
#define STI_DEVICE_MONITORLISTENER_H

#include <sti/utils/MixedValue.h>

#include <string>


namespace STI
{
namespace Device
{

class MonitorListener
{
public:

	virtual ~MonitorListener() {}

	virtual void activated(const std::string& id) = 0;
	virtual void deactivated(const std::string& id) = 0;
	virtual void valueUpdated(const std::string& id, const STI::Utils::MixedValue& value) = 0;
};


} //Device
} //STI

#endif


