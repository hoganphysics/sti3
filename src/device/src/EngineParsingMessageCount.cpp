#include <sti/engine/EngineParsingMessageCount.h>

#include "CerealArchives.h"
#include <cereal/types/map.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/vector.hpp>

using STI::Engine::EngineParsingMessage;
using STI::Engine::ParsingMessageType;
using STI::Engine::EngineParsingMessageCount;


EngineParsingMessageCount::EngineParsingMessageCount()
: errorCount(0), warningCount(0), infoCount(0)
{
}

EngineParsingMessageCount::EngineParsingMessageCount(const std::vector<EngineParsingMessage>& messages)
: errorCount(0), warningCount(0), infoCount(0)
{
    setCounts(messages);
}

EngineParsingMessageCount::~EngineParsingMessageCount()
{
}

void EngineParsingMessageCount::setCounts(const std::vector<EngineParsingMessage>& messages)
{
    errorCount = 0;
    warningCount = 0;
    infoCount = 0;

    for (const auto& message : messages) {
        switch (message.getType()) {
            case ParsingMessageType::Error:
                errorCount++;
                break;
            case ParsingMessageType::Warning:
                warningCount++;
                break;
            case ParsingMessageType::Information:
                infoCount++;
                break;
        }
    }
}


template<class Archive>
void EngineParsingMessageCount::serialize(Archive& archive)
{
    archive( 
        cereal::make_nvp("errorCount", errorCount),
        cereal::make_nvp("warningCount", warningCount),
        cereal::make_nvp("infoCount", infoCount)
        );
}

template void EngineParsingMessageCount::serialize<cereal::XMLOutputArchive>( cereal::XMLOutputArchive& );
template void EngineParsingMessageCount::serialize<cereal::XMLInputArchive>( cereal::XMLInputArchive& );

