
#include "ParsedTag.h"


#include "CerealArchives.h"
#include <cereal/types/common.hpp>
#include <cereal/types/string.hpp>

using STI::Engine::ParsedTag;


bool ParsedTag::operator<(const ParsedTag& rhs) const 
{
    // if (scope == rhs.scope) return name < rhs.name;
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
		// cereal::make_nvp("scope", scope)
		);
}


template void ParsedTag::serialize<cereal::XMLOutputArchive>( cereal::XMLOutputArchive& );
template void ParsedTag::serialize<cereal::XMLInputArchive>( cereal::XMLInputArchive& );
