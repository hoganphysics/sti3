#ifndef STI_ENGINE_EVENTSTATE_H
#define STI_ENGINE_EVENTSTATE_H


namespace STI
{
namespace Engine
{

enum EngineState
{
//	Empty, Clearing, Transferring, Parsed, Loading, Loaded,
//	PreparingToPlay, ReadyToPlay, WaitingForTrigger, Playing, Unknown, Missing, Error, STATES_LENGTH

	Idle, Parsing, Parsed, Arming, WaitingForTrigger, Playing, Paused, Unknown, Missing, Error
	//Missing means the engine does not exist

};


} //Engine
} //STI

#endif
