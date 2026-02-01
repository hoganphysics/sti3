#ifndef STI_NETWORK_REMOTEEVENTENGINEDEPENDENCYPARSER_H
#define STI_NETWORK_REMOTEEVENTENGINEDEPENDENCYPARSER_H

#include "generated/deviceNet.h"
#include "TReferenceHolder.h"
#include <sti/device/DeviceID.h>
#include <sti/device/DeviceTrace.h>
#include <sti/engine/EventEngineDependencyParser.h>

#include <vector>
#include <set>
#include <memory>
#include <mutex>


namespace STI
{
namespace Network
{


class RemoteEventEngineDependencyParser : public STI::Engine::EventEngineDependencyParser,
                                          public STI::TNetwork::TReferenceHolder<STI::TNetwork::TEventEngineDependencyParser>	//mixin
{
public:

    RemoteEventEngineDependencyParser(::STI::TNetwork::TEventEngineDependencyParser_var dependencyParser);
    ~RemoteEventEngineDependencyParser();

    void getDependants(const std::set<STI::Device::DeviceID>& evtTargets, STI::Engine::EventEngineDependencyTree& tree, 
                                std::set<STI::Device::DeviceID>& missingTargets, std::vector<STI::Engine::EngineParsingMessage>& messages, 
                                const STI::Device::DeviceTrace& trace);

    void addDeviceEventTargets(STI::Engine::EventEngineDependencyTree& tree, 
                                std::vector<STI::Engine::EngineParsingMessage>& messages, const STI::Device::DeviceTrace& trace);

    bool ping() const;

private:

    mutable std::mutex dependencyMutex;

};


} //Network
} //STI

#endif
