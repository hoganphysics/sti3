#ifndef STI_ENGINE_PARSEDTAG_H
#define STI_ENGINE_PARSEDTAG_H


#include <sti/engine/StackTrace.h>
#include <sti/device/DeviceID.h>
#include "RawEventGroup.h"

#include <string>


namespace STI
{
namespace Engine
{


class ParsedTag
{
public:

    std::string name;
    STI::Engine::StackTrace trace;
    // STI::Device::DeviceID targetServerID;
    // RawEventGroup scope;

    bool operator<(const ParsedTag& rhs) const;
    bool operator==(const ParsedTag& rhs) const;
    bool operator!=(const ParsedTag& rhs) const;

  	template<class Archive>
	void serialize(Archive& archive);
};


} //Engine
} //STI

#endif
