
#include "EventEngineStateMachine.h"

#include <mutex>

using STI::Engine::EventEngineStateMachine;
using STI::Engine::EngineState;

EventEngineStateMachine::EventEngineStateMachine()
	: state(Idle)
{
	//states
	stateTree.addVertex(Idle);
	stateTree.addVertex(Parsing);
	stateTree.addVertex(Parsed);
//	stateTree.addVertex(Arming);
	stateTree.addVertex(PreparingPlay);
	stateTree.addVertex(PlayReady);	
	stateTree.addVertex(WaitingForTrigger);
	stateTree.addVertex(Playing);

	stateTree.addVertex(Unknown);
	stateTree.addVertex(Missing);
	stateTree.addVertex(Error);

	//allowed transitions: addEdge(source, target)
	stateTree.addEdge(Idle, Parsing);
	stateTree.addEdge(Parsing, Idle);
	stateTree.addEdge(Parsing, Parsed);

	stateTree.addEdge(Parsed, Idle);
	stateTree.addEdge(Parsed, PreparingPlay);
	stateTree.addEdge(PreparingPlay, Parsed);

	stateTree.addEdge(PreparingPlay, PlayReady);
	stateTree.addEdge(PlayReady, WaitingForTrigger);
	stateTree.addEdge(PlayReady, Parsed);

	stateTree.addEdge(WaitingForTrigger, Parsed);
	stateTree.addEdge(WaitingForTrigger, Playing);
	stateTree.addEdge(Playing, Parsed);

	//Error states
	stateTree.addEdge(Unknown, Idle);
	stateTree.addEdge(Missing, Idle);
	stateTree.addEdge(Error, Idle);

	std::vector<EngineState> allStates = { Idle, Parsing, Parsed, PreparingPlay, PlayReady, WaitingForTrigger, Playing };

	//All states can transition to these states
	for (auto s : allStates) {
		stateTree.addEdge(s, Error);
		stateTree.addEdge(s, Unknown);
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
	case Parsing:
		success = _setState(Idle);
		break;

	case PreparingPlay:
	case PlayReady:
	case WaitingForTrigger:
	case Playing:
		success = _setState(Parsed);
		break;
	}

	if (!success) {
		_setState(Unknown);
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

