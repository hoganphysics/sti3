
#include <sti/engine/EventStackTrace.h>

#include <string>
#include <sstream>

#include "CerealArchives.h"
#include <cereal/types/common.hpp>
#include <cereal/types/string.hpp>


using STI::Engine::EventStackTrace;


std::string EventStackTrace::file() const
{ 
    return "";
}

long EventStackTrace::line() const
{
    return 0;
}

std::string EventStackTrace::print(std::string indent) const
{
    //this is a temporary implementation to mock things up
    std::stringstream trace;
    trace << indent << ">>> " << file() << ", line " << line() << "." << std::endl;

    return trace.str();
}


template<class Archive>
void EventStackTrace::serialize(Archive& archive)
{
	// archive(
	// 	cereal::make_nvp("file", file), 
	// 	cereal::make_nvp("line", line)
	// 	);
}

template void EventStackTrace::serialize<cereal::XMLOutputArchive>( cereal::XMLOutputArchive& );
template void EventStackTrace::serialize<cereal::XMLInputArchive>( cereal::XMLInputArchive& );

