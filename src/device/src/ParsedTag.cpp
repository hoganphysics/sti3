
#include <sti/engine/ParsedTag.h>
#include <sti/engine/StackTraceData.h>
#include <sti/engine/RawEventGroup.h>

#include "CerealArchives.h"
#include <cereal/types/common.hpp>
#include <cereal/types/string.hpp>

using STI::Engine::ParsedTag;


ParsedTag::ParsedTag()
{
}

ParsedTag::ParsedTag(const std::string& name, const RawEventGroup* group,
		const STI::Engine::StackTrace& trace, const std::shared_ptr<StackTraceData>& stackTraceData)
: name(name), trace(trace), stackTraceData(stackTraceData), parentGroup(group)
{
}

std::string ParsedTag::getGroupName() const
{
	if (parentGroup != 0) {
		return parentGroup->getFullName();
	}
	return "";
}

bool ParsedTag::operator<(const ParsedTag& rhs) const 
{
    return name < rhs.name;
}

bool ParsedTag::operator==(const ParsedTag& rhs) const 
{
	return name == rhs.name;
}

bool ParsedTag::operator!=(const ParsedTag& rhs) const 
{
    return !((*this) == rhs);
}

template<class Archive>
void ParsedTag::serialize(Archive& archive)
{
	archive(
		cereal::make_nvp("name", name), 
		cereal::make_nvp("trace", trace) 
		);
}


template void ParsedTag::serialize<cereal::XMLOutputArchive>( cereal::XMLOutputArchive& );
template void ParsedTag::serialize<cereal::XMLInputArchive>( cereal::XMLInputArchive& );
