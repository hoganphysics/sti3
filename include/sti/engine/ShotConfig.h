#ifndef STI_ENGINE_SHOTCONFIG_H
#define STI_ENGINE_SHOTCONFIG_H

#include <sti/engine/EngineJobSourceID.h>

#include <string>


namespace STI
{
namespace Engine
{

/*
Shot

ShotConfiguration
ShotInfo
ShotSetup
ShotDefinition
ShotConfig

*/

enum class ShotType { Single, Sequence, SingleUndocumented };

std::string printShotType(const ShotType& type);

class ShotConfig
{
public:

    ShotConfig();

	ShotType shotType;

	EngineJobSourceID jobSourceID;

	int targetEnginePool;

	std::string file;	    //primary file
	std::string comment;	//optional description of this shot

	std::string print() const;

  	template<class Archive>
	void serialize(Archive& archive);
};


} //Engine
} //STI

#endif

