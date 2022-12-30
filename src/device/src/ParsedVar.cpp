
#include <sti/engine/ParsedVar.h>
#include "StackTraceData.h"
#include <sti/engine/RawEventGroup.h>

#include "CerealArchives.h"
#include <cereal/types/common.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/memory.hpp>

#include <sstream>

using STI::Engine::ParsedVar;
using STI::Engine::RawEventGroup;


ParsedVar::ParsedVar()
{
}

ParsedVar::ParsedVar(const std::string& name, const RawEventGroup* group,
		const STI::Engine::StackTrace& trace, const std::shared_ptr<StackTraceData>& stackTraceData)
: name(name), parentGroup(group), trace(trace), stackTraceData(stackTraceData)
{
}

ParsedVar::ParsedVar(const std::string& name, const RawEventGroup* group, const STI::Utils::MixedValue& value, 
		const STI::Engine::StackTrace& trace, const std::shared_ptr<StackTraceData>& stackTraceData)
: name(name), parentGroup(group), value(value), trace(trace), stackTraceData(stackTraceData)
{
}

bool ParsedVar::operator<(const ParsedVar& rhs) const 
{
    // if (scope == rhs.scope) return name < rhs.name;
    // return scope < rhs.scope;

	if (getGroupName() == rhs.getGroupName()) {
		return name < rhs.name;
	}
	return getGroupName() < rhs.getGroupName();

}

bool ParsedVar::operator==(const ParsedVar& rhs) const 
{
    // return (scope == rhs.scope) && (name == rhs.name);
	return (getGroupName() == rhs.getGroupName()) && (name == rhs.name);
}

bool ParsedVar::operator!=(const ParsedVar& rhs) const 
{
    return !((*this) == rhs);
}

bool ParsedVar::isBound() const
{
	return value.getType() != STI::Utils::MixedValueType::Empty;
}

std::string ParsedVar::getGroupName() const
{
	if (parentGroup != 0) {
		return parentGroup->getFullName();
	}
	return "";
}

template<class Archive>
void ParsedVar::serialize(Archive& archive)
{
	archive(
		cereal::make_nvp("name", name), 
		cereal::make_nvp("value", value), 
		cereal::make_nvp("trace", trace),
		// cereal::make_nvp("parentGroup", parentGroup),	//raw pointers not supported
		cereal::make_nvp("stackTraceData", stackTraceData)
		// cereal::make_nvp("scope", scope)
		);
}


template void ParsedVar::serialize<cereal::XMLOutputArchive>( cereal::XMLOutputArchive& );
template void ParsedVar::serialize<cereal::XMLInputArchive>( cereal::XMLInputArchive& );
