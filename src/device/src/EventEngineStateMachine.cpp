
#include "EventEngineStateMachine.h"

#include <mutex>

using STI::Engine::EventEngineStateMachine;
using STI::Engine::EngineState;

EventEngineStateMachine::EventEngineStateMachine()
	: state(EngineState::Idle)
{
	//states
	stateTree.addVertex(EngineState::Idle);
	stateTree.addVertex(EngineState::Parsing);
	stateTree.addVertex(EngineState::Parsed);

	stateTree.addVertex(EngineState::PreparingPlay);
	stateTree.addVertex(EngineState::PlayReady);	
	stateTree.addVertex(EngineState::WaitingForTrigger);
	stateTree.addVertex(EngineState::Playing);

	stateTree.addVertex(EngineState::Unknown);
	stateTree.addVertex(EngineState::Missing);
	stateTree.addVertex(EngineState::Error);

	//allowed transitions: addEdge(source, target)
	stateTree.addEdge(EngineState::Idle, EngineState::Parsing);
	stateTree.addEdge(EngineState::Parsing, EngineState::Idle);
	stateTree.addEdge(EngineState::Parsing, EngineState::Parsed);

	stateTree.addEdge(EngineState::Parsed, EngineState::Idle);
	stateTree.addEdge(EngineState::Parsed, EngineState::PreparingPlay);
	stateTree.addEdge(EngineState::PreparingPlay, EngineState::Parsed);

	stateTree.addEdge(EngineState::PreparingPlay, EngineState::PlayReady);
	stateTree.addEdge(EngineState::PlayReady, EngineState::WaitingForTrigger);
	stateTree.addEdge(EngineState::PlayReady, EngineState::Parsed);

	stateTree.addEdge(EngineState::WaitingForTrigger, EngineState::Parsed);
	stateTree.addEdge(EngineState::WaitingForTrigger, EngineState::Playing);
	stateTree.addEdge(EngineState::Playing, EngineState::Parsed);

	//Error states
	stateTree.addEdge(EngineState::Unknown, EngineState::Idle);
	stateTree.addEdge(EngineState::Missing, EngineState::Idle);
	stateTree.addEdge(EngineState::Error, EngineState::Idle);

	std::vector<EngineState> allStates = { EngineState::Idle, EngineState::Parsing, EngineState::Parsed, EngineState::PreparingPlay, EngineState::PlayReady, EngineState::WaitingForTrigger, EngineState::Playing };

	//All states can transition to these states
	for (auto s : allStates) {
		stateTree.addEdge(s, EngineState::Error);
		stateTree.addEdge(s, EngineState::Unknown);
	}
}

EventEngineStateMachine::~EventEngineStateMachine()
{
}

void EventEngineStateMachine::stop()
{
//	Idle  <-- Clearing, Transferring
//	Parsed <-- Loading
//	Loaded <-- PreparingToPlay, WaitingForTrigger, Playing

	std::unique_lock<std::mutex> ulock(stateMutex);

	bool success = false;

	//using fall through to send groups of "-ing" states to a common static state
	switch (state) {
	case EngineState::Parsing:
		success = _setState(EngineState::Idle);
		break;

	case EngineState::PreparingPlay:
	case EngineState::PlayReady:
	case EngineState::WaitingForTrigger:
	case EngineState::Playing:
		success = _setState(EngineState::Parsed);
		break;
	}

	if (!success) {
		_setState(EngineState::Unknown);
	}
}

bool EventEngineStateMachine::isAllowedTransition(EngineState target) const
{
	std::unique_lock<std::mutex> ulock(stateMutex);
	return _isAllowedTransition(target);
}

bool EventEngineStateMachine::_isAllowedTransition(EngineState target) const
{
	//private use only; mutex protected elsewhere
	return stateTree.isDependedentNode(state, target);
}

bool EventEngineStateMachine::setState(EngineState target)
{
	std::unique_lock<std::mutex> ulock(stateMutex);
	return _setState(target);
}

bool EventEngineStateMachine::_setState(EngineState target)
{
	//private use only; mutex protected elsewhere

	if (_isAllowedTransition(target)) {
		state = target;
	}

	return _isState(target);
}

bool EventEngineStateMachine::isState(EngineState target) const
{
	std::unique_lock<std::mutex> ulock(stateMutex);
	return _isState(target);
}

bool EventEngineStateMachine::_isState(EngineState target) const
{
	//private use only; mutex protected elsewhere
	return state == target;
}

STI::Engine::EngineState EventEngineStateMachine::getState() const
{
	std::unique_lock<std::mutex> ulock(stateMutex);
	return state;
}

