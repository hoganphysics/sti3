#ifndef STI_ENGINE_LOCALEVENTENGINEDEPENDENCYPARSER_H
#define STI_ENGINE_LOCALEVENTENGINEDEPENDENCYPARSER_H

#include <sti/LocalDevice.h>

#include <sti/device/DeviceCollection.h>

#include <sti/engine/EventEngineDependencyParser.h>

#include <vector>
#include <set>
#include <map>
#include <string>


namespace STI
{
namespace Engine
{


class LocalEventEngineDependencyParser : public EventEngineDependencyParser
{
public:

    LocalEventEngineDependencyParser(STI::Device::LocalDevice* localDevice);
    ~LocalEventEngineDependencyParser();

    void getDependants(const std::set<STI::Device::DeviceID>& evtTargets, EventEngineDependencyTree& tree, 
                        std::set<STI::Device::DeviceID>& missingTargets, std::vector<EngineParsingMessage>& messages, 
                        unsigned maxRecursions);

    void getDependants(const std::set<STI::Device::DeviceID>& evtTargets, EventEngineDependencyTree& tree, 
                                std::set<STI::Device::DeviceID>& missingTargets, std::vector<EngineParsingMessage>& messages, 
                                const STI::Device::DeviceTrace& trace);
    
    void addDeviceEventTargets(EventEngineDependencyTree& tree, 
                                        std::vector<EngineParsingMessage>& messages, 
                                        const STI::Device::DeviceTrace& trace);

private:

    bool getTargetDependencyParser(const STI::Device::DeviceID& id, std::shared_ptr<EventEngineDependencyParser>& dependencyParser);


    bool loopDetected(const STI::Device::DeviceTrace& trace, STI::Device::DeviceTrace& newTrace);

    void getServerChainIDs(std::set<STI::Device::DeviceID>& serverIDs);


    void addToTargetsByServer(const std::set<STI::Device::DeviceID>& targets, const EventEngineDependencyTree& tree, 
                                std::map<std::string, std::set<STI::Device::DeviceID>>& targetsByServer);
    
    void getDownstreamIDs(const std::map<std::string, std::set<STI::Device::DeviceID>> targetsByServer, const EventEngineDependencyTree& tree, 
                            std::set<STI::Device::DeviceID>& downstreamIDs);


    void getPartnerDeviceDependants(const STI::Device::DeviceID& partnerID, const std::set<STI::Device::DeviceID>& targets, EventEngineDependencyTree& tree, 
                                std::set<STI::Device::DeviceID>& missingIDs, std::vector<EngineParsingMessage>& messages, const STI::Device::DeviceTrace& trace);


    STI::Device::LocalDevice* localDevice;
    STI::Device::DeviceID localDeviceID;
    std::shared_ptr<STI::Device::DeviceCollection> localCollection;

};


} //Engine
} //STI

#endif
