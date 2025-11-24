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

	EngineJobSourceID() = default;
	EngineJobSourceID(const std::string& user, const std::string& machine) : user(user), machine(machine) {}

	std::string user;
	std::string machine;

	std::string print() const;

	std::string toString() const;
	static EngineJobSourceID fromString(const std::string& jobSourceID);

	template<class Archive>
	void serialize(Archive& archive);
};


} //Engine
} //STI

#endif
