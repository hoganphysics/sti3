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
void ParseResult::serialize(Archive& archive)
{
    archive( 
        cereal::make_nvp("pid", pid),
        cereal::make_nvp("baseEventGroup", baseEventGroup), 
        cereal::make_nvp("parsedDevices", parsedDevices),
        cereal::make_nvp("messages", messages),
        cereal::make_nvp("stackTraceResult", stackTraceResult)
        );
}


template void ParseResult::serialize<cereal::XMLOutputArchive>( cereal::XMLOutputArchive& );
template void ParseResult::serialize<cereal::XMLInputArchive>( cereal::XMLInputArchive& );
