
#include "ParsedVar.h"
#include "StackTraceData.h"

#include "CerealArchives.h"
#include <cereal/types/common.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/memory.hpp>

using STI::Engine::ParsedVar;


ParsedVar::ParsedVar()
{
}

ParsedVar::ParsedVar(const std::string& name, const std::string& groupName,
		const STI::Engine::StackTrace& trace, const std::shared_ptr<StackTraceData>& stackTraceData)
: name(name), fullGroupName(groupName), trace(trace), stackTraceData(stackTraceData)
{
}

ParsedVar::ParsedVar(const std::string& name, const std::string& groupName, const STI::Utils::MixedValue& value, 
		const STI::Engine::StackTrace& trace, const std::shared_ptr<StackTraceData>& stackTraceData)
: name(name), fullGroupName(groupName), value(value), trace(trace), stackTraceData(stackTraceData)
{
}

bool ParsedVar::operator<(const ParsedVar& rhs) const 
{
    // if (scope == rhs.scope) return name < rhs.name;
    // return scope < rhs.scope;

	if (fullGroupName == rhs.fullGroupName) {
		return name < rhs.name;
	}
	return fullGroupName < rhs.fullGroupName;

}

bool ParsedVar::operator==(const ParsedVar& rhs) const 
{
    // return (scope == rhs.scope) && (name == rhs.name);
	return (fullGroupName == rhs.fullGroupName) && (name == rhs.name);
}

bool ParsedVar::operator!=(const ParsedVar& rhs) const 
{
    return !((*this) == rhs);
}

bool ParsedVar::isBound() const
{
	return value.getType() == STI::Utils::MixedValueType::Empty;
}

template<class Archive>
void ParsedVar::serialize(Archive& archive)
{
	archive(
		cereal::make_nvp("name", name), 
		cereal::make_nvp("value", value), 
		cereal::make_nvp("trace", trace),
		cereal::make_nvp("stackTraceData", stackTraceData)
		// cereal::make_nvp("scope", scope)
		);
}


template void ParsedVar::serialize<cereal::XMLOutputArchive>( cereal::XMLOutputArchive& );
template void ParsedVar::serialize<cereal::XMLInputArchive>( cereal::XMLInputArchive& );
