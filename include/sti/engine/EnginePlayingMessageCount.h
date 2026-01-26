#ifndef STI_ENGINE_ENGINEPLAYINGMESSAGECOUNT_H
#define STI_ENGINE_ENGINEPLAYINGMESSAGECOUNT_H

#include <sti/engine/EnginePlayingMessage.h>

#include <vector>


namespace STI
{
namespace Engine
{


class EnginePlayingMessageCount
{
public:

    EnginePlayingMessageCount();
    EnginePlayingMessageCount(const std::vector<EnginePlayingMessage>& messages);
    ~EnginePlayingMessageCount();

    unsigned errorCount;
    unsigned warningCount;
    unsigned infoCount;

    void setCounts(const std::vector<EnginePlayingMessage>& messages);
    void appendCounts(const std::vector<EnginePlayingMessage>& messages);
    void clearCounts();

    template<class Archive>
    void serialize(Archive& archive);

};

} //Engine
} //STI

#endif
