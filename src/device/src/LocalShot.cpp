
#include "LocalShot.h"
#include "RawEventGroup.h"
// #include <sti/engine/ParsedVar.h>
// #include "ParsedTag.h"
#include <sti/engine/RawEvent.h>
// #include "RawEventGroup.h"

using STI::Engine::LocalShot;
using STI::Engine::ShotConfig;
using STI::Engine::RawEventGroup;
// using STI::Engine::ParsedVar;
// using STI::Engine::ParsedTag;
// using STI::Engine::ParseResult;


LocalShot::LocalShot(const ShotConfig& config, const std::shared_ptr<RawEventGroup>& baseGroup)
: shotConfig(config), baseEventGroup(baseGroup)
{
    // baseEventGroup = std::make_shared<RawEventGroup>();
    // events = std::make_shared<std::vector<RawEvent>>();
    // parseResult = std::make_shared<ParseResult>();
}

LocalShot::~LocalShot()
{
}

const ShotConfig& LocalShot::getShotConfig() const
{
    return shotConfig;
}

void LocalShot::getBaseEventGroup(std::shared_ptr<RawEventGroup>& baseGroup)
{
    baseGroup = baseEventGroup;
}

void LocalShot::setBaseEventGroup(const std::shared_ptr<RawEventGroup>& baseGroup)
{
    baseEventGroup = baseGroup;
}



// void LocalShot::setEvents(const std::shared_ptr<std::vector<RawEvent>>& evts)
// {
//     events = evts;
// }

// void LocalShot::getEvents(std::shared_ptr<std::vector<RawEvent>>& evts)
// {
//     evts = events;
// }

// void LocalShot::addEvent(const RawEvent& evt)
// {
//     if (events != 0) {
//         events->push_back(evt);
//     }
// }

// void LocalShot::setParseResult(const std::shared_ptr<STI::Engine::ParseResult>& pResult)
// {
//     parseResult = pResult;
// }

// void LocalShot::getParseResult(std::shared_ptr<STI::Engine::ParseResult>& pResult)
// {
//     pResult = parseResult;
// }

// bool LocalShot::addVar(const ParsedVar& var)
// {
//     auto it = vars.find(var);

//     if (it != vars.end()) {
//         //Error, var already defined
//         return false;
//     }

//     vars.insert(var);

//     return true;
// }

// bool LocalShot::addTag(const ParsedTag& tag)
// {
//     auto it = tags.find(tag);

//     if (it != tags.end()) {
//         //Error, tag already defined
//         return false;
//     }

//     tags.insert(tag);

//     return true;
// }

// std::vector<std::shared_ptr<STI::Utils::FileHolder>> LocalShot::getTimingFiles() const
// {
//     auto files = std::vector<std::shared_ptr<STI::Utils::FileHolder>>(timingFiles.size());

//     for (auto& file : timingFiles) {
//         files.at(file.second) = file.first;
//     }

//     return files;
// }

// std::vector<std::string> LocalShot::getTimingFileNames() const
// {
//     auto filenames = std::vector<std::string>(timingFiles.size());

//     for (auto& file : timingFiles) {
//         if (file.first != 0) {
//             filenames.at(file.second) = file.first->getFilename();
//         }
//         else {
//             filenames.at(file.second) = std::string("<missing>");
//         }
//     }
//     return filenames;
// }

// std::vector<std::string> LocalShot::getFunctionNames() const
// {
//     auto names = std::vector<std::string>(functionNames.size());

//     for (auto& name : functionNames) {
//         names.at(name.second) = name.first;
//     }

//     return names;
// }

// std::vector<STI::Engine::RawEventGroup> LocalShot::getGroups()
// {
//     // auto groups = std::vector<RawEventGroup>(eventGroups.size());
//     parseResult->eventGroups.clear();

//     for (auto& group : eventGroups) {
//         parseResult->eventGroups.at(group.second) = (group.first);
//     }

//     return parseResult->eventGroups;
// }


// std::vector<ParsedVar> LocalShot::getParsedVars()
// {
//     // std::vector<ParsedVar> parsedVars;

//     parseResult->parsedVars.clear();

//     for (auto& v : vars) {
//         parseResult->parsedVars.push_back(v);
//     }
//     return parseResult->parsedVars;
// }

// std::vector<ParsedTag> LocalShot::getParsedTags()
// {
//     // std::vector<ParsedTag> parsedTags;

//     parseResult->parsedTags.clear();

//     for (auto& tag : tags) {
//         parseResult->parsedTags.push_back(tag);
//     }
//     return parseResult->parsedTags;
// }

