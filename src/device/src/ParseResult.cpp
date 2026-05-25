#include <sti/engine/ParseResult.h>

#include <sti/engine/ParsedDependencyTree.h>
#include <sti/engine/ParseID.h>
#include <sti/engine/RawEvent.h>
#include <sti/engine/RawEventGroup.h>
#include <sti/engine/StackTraceData.h>
#include <sti/engine/StackTraceResult.h>
#include <sti/utils/FileHolder.h>

#include "CerealArchives.h"
#include <cereal/types/map.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/vector.hpp>

using STI::Engine::ParseResult;


ParseResult::ParseResult()
{
    baseEventGroup = std::make_shared<RawEventGroup>();
    parsedDevices = std::make_shared<ParsedDependencyTree>();
    stackTraceResult = std::make_shared<StackTraceResult>();
}

ParseResult::~ParseResult()
{
}

void ParseResult::deleteFiles(ParseResult& parsedResult, const std::shared_ptr<STI::Utils::FileServer>& fileServer)
{
    if (fileServer == 0) return;
    if (parsedResult.stackTraceResult != 0 && parsedResult.stackTraceResult->stackTraceData != 0) {
        auto& timingFiles = parsedResult.stackTraceResult->stackTraceData->getTimingFiles();

        for (auto& fileID : timingFiles) {
            fileServer->deleteFile(fileID);
        }     
    }
}

template<class Archive>
void ParseResult::save(Archive& archive) const
{
    archive( 
        cereal::make_nvp("pid", pid),
        cereal::make_nvp("shotConfig", shotConfig),
        cereal::make_nvp("baseEventGroup", baseEventGroup), 
        cereal::make_nvp("parsedDevices", parsedDevices),
        cereal::make_nvp("messages", messages),
        cereal::make_nvp("stackTraceResult", stackTraceResult),
        cereal::make_nvp("jobOwner", jobOwner)
        );
}

template<class Archive>
void ParseResult::load(Archive& archive)
{
    archive(
        cereal::make_nvp("pid", pid),
        cereal::make_nvp("shotConfig", shotConfig),
        cereal::make_nvp("baseEventGroup", baseEventGroup),
        cereal::make_nvp("parsedDevices", parsedDevices),
        cereal::make_nvp("messages", messages),
        cereal::make_nvp("stackTraceResult", stackTraceResult)
        );

    jobOwner = STI::Device::DeviceID();
    try {
        archive(cereal::make_nvp("jobOwner", jobOwner));
    }
    catch (const cereal::Exception&) {
        jobOwner = STI::Device::DeviceID();
    }
}

template void ParseResult::save<cereal::XMLOutputArchive>( cereal::XMLOutputArchive& ) const;
template void ParseResult::load<cereal::XMLInputArchive>( cereal::XMLInputArchive& );
