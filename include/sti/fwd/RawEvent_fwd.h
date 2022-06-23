#ifndef STI_ENGINE_RAWEVENT_FWD_H
#define STI_ENGINE_RAWEVENT_FWD_H

#include "DeviceID_fwd.h"

#include <vector>
#include <map>
#include <memory>

namespace STI
{
namespace Engine
{

class RawEvent;
typedef std::vector<RawEvent> RawEventVector;
typedef std::map<double, RawEventVector> RawEventMap;

typedef std::map<STI::Device::DeviceID, RawEventVector> DeviceEventMap;
typedef std::shared_ptr<DeviceEventMap> DeviceEventMap_ptr;

enum class RawEventType { Play, Measurement, Waveform, Pause, Jump };	//...  Normal?


} //Engine
} //STI

#endif
