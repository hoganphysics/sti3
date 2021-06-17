
#include "EngineState.h"

using STI::Engine::EngineState;

std::string STI::Engine::print(const EngineState& state) {
	std::string name = "";

	switch(state) {
		case EngineState::Idle:
		name = "Idle";
		break;
		case EngineState::Parsing:
		name = "Parsing";
		break;
		case EngineState::Parsed:
		name = "Parsed";
		break;
		case EngineState::PreparingPlay:
		name = "PreparingPlay";
		break;
		case EngineState::PlayReady:
		name = "PlayReady";
		break;
		case EngineState::WaitingForTrigger:
		name = "WaitingForTrigger";
		break;
		case EngineState::Playing:
		name = "Playing";
		break;
		case EngineState::Paused:
		name = "Paused";
		break;
		case EngineState::Unknown:
		name = "Unknown";
		break;
		case EngineState::Missing:
		name = "Missing";
		break;
		case EngineState::Error:
		name = "Error";
		break;
		default:
		name = "Unknown";
		break;
	}
	return name;
}

