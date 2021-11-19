#ifndef STI_ENGINE_ENGINEJOBSOURCEID_H
#define STI_ENGINE_ENGINEJOBSOURCEID_H

#include <string>


namespace STI
{
namespace Engine
{


class EngineJobSourceID
{
public:
	std::string user;
	std::string machine;

	template<class Archive>
	void serialize(Archive& archive);
};


} //Engine
} //STI

#endif
