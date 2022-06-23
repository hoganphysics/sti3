

#include "Convert_DeviceTrace.h"
#include <sti/device/DeviceTrace.h>


using STI::Device::DeviceTrace;
using STI::TNetwork::TDeviceTrace;




//DeviceTrace
template<> 
DeviceTrace STI::Network::convert<TDeviceTrace, DeviceTrace>(const TDeviceTrace& tDeviceTrace)
{
    DeviceTrace trace;

    for (unsigned i = 0; i < tDeviceTrace.ids.length(); ++i) {
        trace.addID( convert<STI::TNetwork::TDeviceID, STI::Device::DeviceID>(tDeviceTrace.ids[i]) );
    }

	return trace;
}

template<>
TDeviceTrace STI::Network::convert<DeviceTrace, TDeviceTrace>(const DeviceTrace& deviceTrace)
{
    TDeviceTrace tTrace;
    convert<STI::Device::DeviceID, STI::TNetwork::TDeviceID>(deviceTrace.getIDs(), tTrace.ids);
	return tTrace;
}

