
#include "NetworkShotWrapper.h"
#include "ORBManager.h"
#include "RawEventGroup.h"
#include "ParsedVar.h"
#include "ParsedTag.h"
#include <sti/utils/FileHolder.h>


using STI::Network::NetworkShotWrapper;


NetworkShotWrapper::NetworkShotWrapper(const std::shared_ptr<STI::Engine::Shot>& shot)
: localshot(shot), shotEventsCBServant(shot)
{
    STI::Network::ORBManager::ORBManager::activateServant(shotEventsCBServant);

    if (localshot != 0) {
        shotConfig = localshot->getShotConfig();
    }
}

NetworkShotWrapper::~NetworkShotWrapper()
{
}

const STI::Engine::ShotConfig& NetworkShotWrapper::getShotConfig() const
{
    return shotConfig;
}

void NetworkShotWrapper::getBaseEventGroup(std::shared_ptr<STI::Engine::RawEventGroup>& baseGroup)
{
    if (localshot != 0) {
        localshot->getBaseEventGroup(baseGroup);
    }
}

// void NetworkShotWrapper::getEvents(std::shared_ptr<std::vector<STI::Engine::RawEvent>>& evts)
// {
//     if (localshot != 0) {
//         localshot->getEvents(evts);
//     }
// }

// void NetworkShotWrapper::getParseResult(std::shared_ptr<STI::Engine::ParseResult>& parseResult)
// {
//     if (localshot != 0) {
//         localshot->getParseResult(parseResult);
//     }
// }

// void NetworkShotWrapper::setParseResult(const std::shared_ptr<STI::Engine::ParseResult>& parseResult)
// {
//     if (localshot != 0) {
//         localshot->setParseResult(parseResult);
//     }
// }

bool NetworkShotWrapper::getTShotReference(STI::TNetwork::TShotCallback_ptr& tShotCallback)
{
    tShotCallback = shotEventsCBServant._this();
    return !CORBA::is_nil(tShotCallback);
}

// std::vector<std::shared_ptr<STI::Utils::FileHolder>> NetworkShotWrapper::getTimingFiles() const
// {
//     if (localshot != 0) {
//         return localshot->getTimingFiles();
//     }

//     std::vector<std::shared_ptr<STI::Utils::FileHolder>> files;
//     return files;
// }

// std::vector<std::string> NetworkShotWrapper::getTimingFileNames() const
// {
//     if (localshot != 0) {
//         return localshot->getTimingFileNames();
//     }

//     std::vector<std::string> filenames;
//     return filenames;
// }

// std::vector<std::string> NetworkShotWrapper::getFunctionNames() const
// {
//     if (localshot != 0) {
//         return localshot->getFunctionNames();
//     }

//     std::vector<std::string> functions;
//     return functions;
// }

// // std::vector<STI::Engine::RawEventGroup> NetworkShotWrapper::getGroups()
// std::vector<STI::Engine::RawEventGroup> NetworkShotWrapper::getGroups()
// {
//     if (localshot != 0) {
//         return localshot->getGroups();
//     }

//     std::vector<STI::Engine::RawEventGroup> groups;
//     return groups;
// }

// std::vector<STI::Engine::ParsedVar> NetworkShotWrapper::getParsedVars()
// {
//     if (localshot != 0) {
//         return localshot->getParsedVars();
//     }

//     std::vector<STI::Engine::ParsedVar> vars;
//     return vars;
// }

// std::vector<STI::Engine::ParsedTag> NetworkShotWrapper::getParsedTags()
// {
//     if (localshot != 0) {
//         return localshot->getParsedTags();
//     }

//     std::vector<STI::Engine::ParsedTag> tags;
//     return tags;
// }

