#ifndef STI_ENGINE_ENGINEPARSINGMESSAGECOUNT_H
#define STI_ENGINE_ENGINEPARSINGMESSAGECOUNT_H

#include <sti/engine/EngineParsingMessage.h>

#include <vector>


namespace STI
{
namespace Engine
{


class EngineParsingMessageCount
{
public:

    EngineParsingMessageCount();
    EngineParsingMessageCount(const std::vector<EngineParsingMessage>& messages);
    ~EngineParsingMessageCount();

    unsigned errorCount;
    unsigned warningCount;
    unsigned infoCount;

    void setCounts(const std::vector<EngineParsingMessage>& messages);

    template<class Archive>
    void serialize(Archive& archive);

};

} //Engine
} //STI

#endif
