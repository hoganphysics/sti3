#ifndef STI_ENGINE_LOCALSHOT_H
#define STI_ENGINE_LOCALSHOT_H

#include "Shot.h"
#include <sti/engine/ShotConfig.h>
#include "ParseResult.h"
#include <sti/utils/utils.h>

#include <vector>
#include <memory>
#include <map>
#include <string>
#include <set>


namespace STI
{
namespace Engine
{

class RawEvent;
class RawEventGroup;


class LocalShot : public Shot
{
public:

    LocalShot(const ShotConfig& config);
    virtual ~LocalShot();

    const ShotConfig& getShotConfig() const;

    void setEvents(const std::shared_ptr<std::vector<RawEvent>>& evts);
    void getEvents(std::shared_ptr<std::vector<RawEvent>>& evts);

    void setParseResult(const std::shared_ptr<STI::Engine::ParseResult>& pResult);
    void getParseResult(std::shared_ptr<STI::Engine::ParseResult>& pResult);

    void addEvent(const RawEvent& evt);
    bool addVar(const ParsedVar& var);
    bool addTag(const ParsedTag& tag);

    std::vector<std::shared_ptr<STI::Utils::FileHolder>> getTimingFiles() const;

    std::vector<std::string> getTimingFileNames() const;
    std::vector<std::string> getFunctionNames() const;
    std::vector<RawEventGroup> getGroups();
    std::vector<ParsedVar> getParsedVars();
    std::vector<ParsedTag> getParsedTags();

private:

    ShotConfig shotConfig;

    std::shared_ptr<std::vector<RawEvent>> events;
    std::shared_ptr<STI::Engine::ParseResult> parseResult;

    std::map<std::shared_ptr<STI::Utils::FileHolder>, unsigned> timingFiles;

    std::map<std::string, unsigned> functionNames;
    std::map<STI::Engine::RawEventGroup, unsigned, STI::Utils::shared_ptr_Comparator<RawEventGroup>> eventGroups;

    std::set<ParsedVar> vars;
    std::set<ParsedTag> tags;

    // ParseResult parseResult;    //caching

//    std::vector<AbstractEvent> abstractevents;
    //files
    //overwritten vars
    //abstract channel resolution (?)

};


} //Engine
} //STI

#endif
