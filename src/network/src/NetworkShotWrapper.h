#ifndef STI_ENGINE_NETWORKSHOTWRAPPER_H
#define STI_ENGINE_NETWORKSHOTWRAPPER_H

#include "Shot.h"
#include "TShotRefInterface.h"

#include "TShotCallback_i.h"
#include "deviceNet.h"
#include <sti/engine/ShotConfig.h>

#include <vector>
#include <memory>


namespace STI
{
namespace Network
{


class NetworkShotWrapper : public STI::Engine::Shot,
                           public STI::Network::TShotRefInterface	//mixin
{
public:

    NetworkShotWrapper(const std::shared_ptr<STI::Engine::Shot>& shot);
    ~NetworkShotWrapper();

    const STI::Engine::ShotConfig& getShotConfig() const;

    void getBaseEventGroup(std::shared_ptr<STI::Engine::RawEventGroup>& baseGroup);


    // void getEvents(std::shared_ptr<std::vector<STI::Engine::RawEvent>>& evts);
    // void getParseResult(std::shared_ptr<STI::Engine::ParseResult>& parseResult);
    // void setParseResult(const std::shared_ptr<STI::Engine::ParseResult>& parseResult);

    // std::vector<std::shared_ptr<STI::Utils::FileHolder>> getTimingFiles() const;

    // std::vector<std::string> getTimingFileNames() const;
    // std::vector<std::string> getFunctionNames() const;

    // std::vector<STI::Engine::RawEventGroup> getGroups();
    // std::vector<STI::Engine::ParsedVar> getParsedVars();
    // std::vector<STI::Engine::ParsedTag> getParsedTags();

private:

    bool getTShotReference(STI::TNetwork::TShotCallback_ptr& tShotCallback);

    STI::Engine::ShotConfig shotConfig;

    std::shared_ptr<STI::Engine::Shot> localshot;
    STI::TNetwork::TShotCallback_i shotEventsCBServant;

};


} //Network
} //STI

#endif
