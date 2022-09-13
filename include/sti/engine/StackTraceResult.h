#ifndef STI_ENGINE_STACKTRACERESULT_H
#define STI_ENGINE_STACKTRACERESULT_H

#include <sti/engine/ParseID.h>

#include <memory>


namespace STI
{
namespace Engine
{

class StackTraceData;


class StackTraceResult
{
public:

    StackTraceResult();
    StackTraceResult(const ParseID& pid);

    ParseID pid;

    std::shared_ptr<StackTraceData> stackTraceData;

    template<class Archive>
    void serialize(Archive& archive);

private:

};

} //Engine
} //STI

#endif
