#ifndef STI_ENGINE_PARSEDTAG_H
#define STI_ENGINE_PARSEDTAG_H

#include <sti/engine/CompressedStackTrace.h>
#include <sti/device/DeviceID.h>

#include <sti/engine/RawEventGroup.h>

#include <string>
#include <memory>


namespace STI
{
namespace Engine
{

class RawEventGroup;    
class StackTraceData;


class ParsedTag
{
public:

    ParsedTag();
    ParsedTag(const std::string& name, const RawEventGroup* group,
            const STI::Engine::CompressedStackTrace& trace, const std::shared_ptr<StackTraceData>& stackTraceData);

    std::string name;
    STI::Engine::CompressedStackTrace trace;
    
    std::shared_ptr<StackTraceData> stackTraceData;

    const RawEventGroup* parentGroup;

    std::string getGroupName() const;

    bool operator<(const ParsedTag& rhs) const;
    bool operator==(const ParsedTag& rhs) const;
    bool operator!=(const ParsedTag& rhs) const;

  	template<class Archive>
	  void serialize(Archive& archive);
};


} //Engine
} //STI

#endif
