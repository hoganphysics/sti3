#ifndef STI_ENGINE_ENGINESTATE_H
#define STI_ENGINE_ENGINESTATE_H

#include <string>

namespace STI
{
namespace Engine
{

enum class EngineState
{
//	Empty, Clearing, Transferring, Parsed, Loading, Loaded,
//	PreparingToPlay, ReadyToPlay, WaitingForTrigger, Playing, Unknown, Missing, Error, STATES_LENGTH

	//removed: Arming

	Idle, Parsing, Parsed, PreparingPlay, PlayReady, WaitingForTrigger, Playing, Paused, Unknown, Missing, Error
	//Missing means the engine does not exist

};

std::string print(const EngineState& state);

} //Engine
} //STI

#endif
