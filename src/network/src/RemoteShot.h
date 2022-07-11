#ifndef STI_NETWORK_REMOTESHOT_H
#define STI_NETWORK_REMOTESHOT_H

#include "deviceNet.h"

#include <sti/engine/ShotConfig.h>
#include "Shot.h"
#include "TReferenceHolder.h"
#include "TShotRefInterface.h"
#include <sti/utils/CachedValue.h>

#include <memory>
#include <vector>


namespace STI
{
namespace Network
{

class RemoteShot : public STI::Engine::Shot,
                   public STI::TNetwork::TReferenceHolder<STI::TNetwork::TShotCallback>,	//mixin
                   public STI::Network::TShotRefInterface	//mixin
{
public:

	RemoteShot(const STI::Engine::ShotConfig& shotConfig, 
                ::STI::TNetwork::TShotCallback_ptr shotCallback);
    ~RemoteShot();

    void getEvents(std::shared_ptr<std::vector<STI::Engine::RawEvent>>& events);
    void getParseResult(std::shared_ptr<STI::Engine::ParseResult>& pResult);
    void setParseResult(const std::shared_ptr<STI::Engine::ParseResult>& pResult);

    const STI::Engine::ShotConfig& getShotConfig() const;
 
    // void setTimingFiles(const std::vector<std::shared_ptr<STI::Utils::FileHolder>>& files);
    // void setFilenames(const std::vector<std::string>& filenames);   //optimization to avoid extra network calls
    // void setFunctionNames(const std::vector<std::string>& functions);

    std::vector<std::shared_ptr<STI::Utils::FileHolder>> getTimingFiles() const;

    std::vector<std::string> getTimingFileNames() const;
    std::vector<std::string> getFunctionNames() const;

    std::vector<STI::Engine::RawEventGroup> getGroups();
    std::vector<STI::Engine::ParsedVar> getParsedVars();
    std::vector<STI::Engine::ParsedTag> getParsedTags();

private:

    bool getTShotReference(STI::TNetwork::TShotCallback_ptr& tShotCallback);

    void refresh();
    bool refreshRequired;
    
    bool refreshEvents();
    bool refreshParseResult();
    // bool refreshGroups();
    // bool refreshVars();
    // bool refreshTags();

    STI::Engine::ShotConfig shotConfig;
    std::shared_ptr<std::vector<STI::Engine::RawEvent>> storedEvents;
    std::shared_ptr<STI::Engine::ParseResult> parseResult;

    // std::vector<std::shared_ptr<STI::Utils::FileHolder>> timingFiles;
    // std::vector<std::string> timingFileNames;
    // std::vector<std::string> functionNames;

    // //Cached values are mutable since only the servant's values are const
    // mutable STI::Utils::CachedValue<std::vector<STI::Engine::RawEventGroup>> parsedGroups;
    // mutable STI::Utils::CachedValue<std::vector<STI::Engine::ParsedVar>> parsedVars;
    // mutable STI::Utils::CachedValue<std::vector<STI::Engine::ParsedTag>> parsedTags;

    //::STI::TNetwork::TShot_var _tShot;    //remote reference
    mutable std::mutex shotMutex;
};


} //Network
} //STI


#endif

