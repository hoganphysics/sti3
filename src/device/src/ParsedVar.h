#ifndef STI_ENGINE_PARSEDVAR_H
#define STI_ENGINE_PARSEDVAR_H


#include <sti/utils/MixedValue.h>
#include <sti/engine/StackTrace.h>
#include <sti/device/DeviceID.h>

#include "RawEventGroup.h"

#include <string>


namespace STI
{
namespace Engine
{


class ParsedVar
{
public:
    
    std::string name;
    STI::Utils::MixedValue value;
    STI::Engine::StackTrace trace;
    // STI::Device::DeviceID targetServerID;
    RawEventGroup scope;

    bool operator<(const ParsedVar& rhs) const;
    bool operator==(const ParsedVar& rhs) const;
    bool operator!=(const ParsedVar& rhs) const;

   	template<class Archive>
	void serialize(Archive& archive);
};


} //Engine
} //STI

#endif
