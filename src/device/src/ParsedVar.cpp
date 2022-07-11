
#include "ParsedVar.h"


#include "CerealArchives.h"
#include <cereal/types/common.hpp>
#include <cereal/types/string.hpp>

using STI::Engine::ParsedVar;


bool ParsedVar::operator<(const ParsedVar& rhs) const 
{
    if (scope == rhs.scope) return name < rhs.name;
    return scope < rhs.scope;
}

bool ParsedVar::operator==(const ParsedVar& rhs) const 
{
    return (scope == rhs.scope) && (name == rhs.name);
}

bool ParsedVar::operator!=(const ParsedVar& rhs) const 
{
    return !((*this) == rhs);
}

template<class Archive>
void ParsedVar::serialize(Archive& archive)
{
	archive(
		cereal::make_nvp("name", name), 
		cereal::make_nvp("value", value), 
		cereal::make_nvp("trace", trace), 
		cereal::make_nvp("scope", scope)
		);
}


template void ParsedVar::serialize<cereal::XMLOutputArchive>( cereal::XMLOutputArchive& );
template void ParsedVar::serialize<cereal::XMLInputArchive>( cereal::XMLInputArchive& );
