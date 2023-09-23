
#ifndef STI_ENGINE_PARSERESULT_H
#define STI_ENGINE_PARSERESULT_H


#include <sti/engine/ParseID.h>
#include <sti/engine/EngineParsingMessage.h>
#include <sti/fwd/RawEvent_fwd.h>
#include <sti/utils/FileServer.h>

#include <vector>
#include <memory>

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

    static void deleteFiles(ParseResult& parsedResult, const std::shared_ptr<STI::Utils::FileServer>& fileServer);

    template<class Archive>
    void serialize(Archive& archive);

};


} //Engine
} //STI

#endif

