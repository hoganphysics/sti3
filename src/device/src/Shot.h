#ifndef STI_ENGINE_SHOT_H
#define STI_ENGINE_SHOT_H


#include <sti/utils/FileHolder.h>

#include <vector>
#include <memory>


namespace STI
{
namespace Engine
{

class RawEvent;
class RawEventGroup;
class ShotConfig;
class ParsedVar;
class ParsedTag;
class ParseResult;

//Support information for StackTrace
//
//map<unsigned, FileHolder>  [index, file], relative vs absolute path?
//map<unsigned, std::string> [index, function name]



class Shot
{
public:

    virtual ~Shot() {}

    virtual const ShotConfig& getShotConfig() const = 0;
    virtual void getEvents(std::shared_ptr<std::vector<RawEvent>>& evts) = 0;
    virtual void getParseResult(std::shared_ptr<STI::Engine::ParseResult>& parseResult) = 0;
    virtual void setParseResult(const std::shared_ptr<STI::Engine::ParseResult>& parseResult) = 0;

    virtual std::vector<std::shared_ptr<STI::Utils::FileHolder>> getTimingFiles() const = 0;

    virtual std::vector<std::string> getTimingFileNames() const = 0;
    virtual std::vector<std::string> getFunctionNames() const = 0;

    virtual std::vector<RawEventGroup> getGroups() = 0;
    virtual std::vector<ParsedVar> getParsedVars() = 0;
    virtual std::vector<ParsedTag> getParsedTags() = 0;

private:

//    std::vector<RawEvent> events;

//    std::vector<AbstractEvent> abstractevents;
    //files
    //overwritten vars
    //abstract channel resolution (?)
};


} //Engine
} //STI

#endif
