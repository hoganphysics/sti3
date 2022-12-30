#ifndef STI_ENGINE_EVENTENGINEDEPENDENCYPARSER_H
#define STI_ENGINE_EVENTENGINEDEPENDENCYPARSER_H

#include <sti/device/DeviceID.h>
#include <sti/device/DeviceTrace.h>

#include <vector>
#include <set>


namespace STI
{
namespace Engine
{

class EventEngineDependencyTree;
class EngineParsingMessage;

class EventEngineDependencyParser
{
public:

    virtual ~EventEngineDependencyParser() {}

    virtual void getDependants(const std::set<STI::Device::DeviceID>& evtTargets, EventEngineDependencyTree& tree, 
                                std::set<STI::Device::DeviceID>& missingTargets, std::vector<EngineParsingMessage>& messages, 
                                const STI::Device::DeviceTrace& trace) = 0;
    
    virtual void addDeviceEventTargets(EventEngineDependencyTree& tree, 
                                        std::vector<EngineParsingMessage>& messages, 
                                        const STI::Device::DeviceTrace& trace) = 0;

};


} //Engine
} //STI

#endif
