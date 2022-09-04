#ifndef STI_ENGINE_PARSEDVAR_H
#define STI_ENGINE_PARSEDVAR_H


#include <sti/utils/MixedValue.h>
#include <sti/engine/StackTrace.h>
#include <sti/device/DeviceID.h>

#include <string>


namespace STI
{
namespace Engine
{

class RawEventGroup;
class StackTraceData;


class ParsedVar
{
public:

    ParsedVar();
    ParsedVar(const std::string& name, const RawEventGroup* group,
            const STI::Engine::StackTrace& trace, const std::shared_ptr<StackTraceData>& stackTraceData);
    ParsedVar(const std::string& name, const RawEventGroup* group, const STI::Utils::MixedValue& value, 
            const STI::Engine::StackTrace& trace, const std::shared_ptr<StackTraceData>& stackTraceData);
    
    std::string name;
//     std::string fullGroupName;
    STI::Utils::MixedValue value;   //can be MixedValueType::Empty to indicate an unbound var
    
    STI::Engine::StackTrace trace;
    std::shared_ptr<StackTraceData> stackTraceData;

    const RawEventGroup* parentGroup;

    std::string getGroupName() const;
    bool isBound() const;

    bool operator<(const ParsedVar& rhs) const;
    bool operator==(const ParsedVar& rhs) const;
    bool operator!=(const ParsedVar& rhs) const;
    
    template<class Archive>
    void serialize(Archive& archive);

};


} //Engine
} //STI

#endif
