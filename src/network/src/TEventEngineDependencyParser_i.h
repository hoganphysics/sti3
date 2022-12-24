#ifndef STI_TNETWORK_TEVENTENGINEDEPENDENCYPARSER_I_H
#define STI_TNETWORK_TEVENTENGINEDEPENDENCYPARSER_I_H

#include <sti/engine/EventEngineScheduler.h>
#include <sti/device/Device.h>

#include "deviceNet.h"

#include <memory>


namespace STI
{
namespace TNetwork
{


class TEventEngineDependencyParser_i : public POA_STI::TNetwork::TEventEngineDependencyParser
{
public:

	TEventEngineDependencyParser_i(const std::shared_ptr<STI::Engine::EventEngineDependencyParser>& dependencyParser);
	~TEventEngineDependencyParser_i();

    void getDependants(const ::STI::TNetwork::TDeviceIDSeq& evtTargets, 
                        ::STI::TNetwork::TEventEngineDependencyTree& tree, 
                        ::STI::TNetwork::TDeviceIDSeq& missingTargets, 
                        ::STI::TNetwork::TEngineParsingMessageSeq_out messages, 
                        const ::STI::TNetwork::TDeviceTrace& trace);
    
    void addDeviceEventTargets(::STI::TNetwork::TEventEngineDependencyTree& tree, 
                                ::STI::TNetwork::TEngineParsingMessageSeq_out messages, 
                                const ::STI::TNetwork::TDeviceTrace& trace);

    ::CORBA::Boolean ping();

private:

    std::shared_ptr<STI::Engine::EventEngineDependencyParser> localDependencyParser;

};

} //TNetwork
} //STI

#endif
