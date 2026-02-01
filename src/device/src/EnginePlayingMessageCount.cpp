#include <sti/engine/EnginePlayingMessageCount.h>

#include "CerealArchives.h"
#include <cereal/types/map.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/vector.hpp>

using STI::Engine::EnginePlayingMessage;
using STI::Engine::EnginePlayingMessage;
using STI::Engine::EnginePlayingMessageCount;


EnginePlayingMessageCount::EnginePlayingMessageCount()
: errorCount(0), warningCount(0), infoCount(0)
{
}

EnginePlayingMessageCount::EnginePlayingMessageCount(const std::vector<EnginePlayingMessage>& messages)
: errorCount(0), warningCount(0), infoCount(0)
{
    setCounts(messages);
}

EnginePlayingMessageCount::~EnginePlayingMessageCount()
{
}

void EnginePlayingMessageCount::clearCounts() 
{
    errorCount = 0;
    warningCount = 0;
    infoCount = 0;
}

void EnginePlayingMessageCount::setCounts(const std::vector<EnginePlayingMessage>& messages)
{
    clearCounts();
    appendCounts(messages);
}

void EnginePlayingMessageCount::appendCounts(const std::vector<EnginePlayingMessage>& messages)
{
    for (const auto& message : messages) {
        switch (message.getType()) {
            case PlayingMessageType::Error:
                errorCount++;
                break;
            case PlayingMessageType::Warning:
                warningCount++;
                break;
            case PlayingMessageType::Information:
                infoCount++;
                break;
        }
    }
}


template<class Archive>
void EnginePlayingMessageCount::serialize(Archive& archive)
{
    archive( 
        cereal::make_nvp("errorCount", errorCount),
        cereal::make_nvp("warningCount", warningCount),
        cereal::make_nvp("infoCount", infoCount)
        );
}

template void EnginePlayingMessageCount::serialize<cereal::XMLOutputArchive>( cereal::XMLOutputArchive& );
template void EnginePlayingMessageCount::serialize<cereal::XMLInputArchive>( cereal::XMLInputArchive& );

