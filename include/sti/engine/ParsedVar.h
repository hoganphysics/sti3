#ifndef STI_ENGINE_PARSEDVAR_H
#define STI_ENGINE_PARSEDVAR_H


#include <sti/utils/MixedValue.h>
#include <sti/engine/CompressedStackTrace.h>
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
            const STI::Engine::CompressedStackTrace& trace, const std::shared_ptr<StackTraceData>& stackTraceData);
    ParsedVar(const std::string& name, const RawEventGroup* group, const STI::Utils::MixedValue& value, 
            const STI::Engine::CompressedStackTrace& trace, const std::shared_ptr<StackTraceData>& stackTraceData);
    
    std::string name;
    STI::Utils::MixedValue value;   //can be MixedValueType::Empty to indicate an unbound var
    
    STI::Engine::CompressedStackTrace trace;
    std::shared_ptr<StackTraceData> stackTraceData;

    std::string getGroupName() const;
    bool isBound() const;

    bool operator<(const ParsedVar& rhs) const;
    bool operator==(const ParsedVar& rhs) const;
    bool operator!=(const ParsedVar& rhs) const;
    
    void refreshGroupName();
    void setParentGroup(const RawEventGroup* group);
    void setGroupName(const std::string& groupName);

    template<class Archive>
    void serialize(Archive& archive);

private:

    std::string _groupName;
    const RawEventGroup* parentGroup;

};


} //Engine
} //STI

#endif
