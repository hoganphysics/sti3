
#ifndef STI_ENGINE_PARSERESULT_H
#define STI_ENGINE_PARSERESULT_H


#include <sti/engine/ParseID.h>
#include <sti/engine/EngineParsingMessage.h>

// #include <sti/engine/ParsedVar.h>
// #include "ParsedTag.h"
#include <sti/fwd/RawEvent_fwd.h>

#include <sti/utils/FileHolder.h>



#include <vector>
#include <memory>
#include <string>



namespace STI
{
namespace Engine
{

class StackTraceResult;
class RawEventGroup;
class ParsedDependencyTree;


class ParseResult
{
public:

    ParseResult();
    virtual ~ParseResult();

    ParseID pid;

    std::shared_ptr<RawEventGroup> baseEventGroup;
    std::shared_ptr<ParsedDependencyTree> parsedDevices;
    std::vector<EngineParsingMessage> messages;
    std::shared_ptr<StackTraceResult> stackTraceResult;



    // std::vector<std::shared_ptr<STI::Utils::FileHolder>> timingFiles;
    // std::vector<std::string> timingFileNames;
    
    // std::vector<std::string> functionNames;
    
    // std::vector<STI::Engine::RawEventGroup> eventGroups;
    // std::vector<ParsedVar> parsedVars;
    // std::vector<ParsedTag> parsedTags;

    // EngineParseResult engineParseResult;

    template<class Archive>
    void serialize(Archive& archive);

};


} //Engine
} //STI

#endif

