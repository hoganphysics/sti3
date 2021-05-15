#ifndef STI_ENGINE_EVENTENGINESTATEMACHINE_H
#define STI_ENGINE_EVENTENGINESTATEMACHINE_H

#include "EngineState.h"
#include "utils/DependencyTree.h"

#include <mutex>


namespace STI
{
namespace Engine
{


class EventEngineStateMachine
{
public:

	EventEngineStateMachine();
	~EventEngineStateMachine();

	bool isAllowedTransition(EngineState target) const;
	bool setState(EngineState target);
	bool isState(EngineState target) const;
	EngineState getState() const;

	void stop();

private:

	bool _isAllowedTransition(EngineState target) const;
	bool _isState(EngineState target) const;
	bool _setState(EngineState target);

	EngineState state;
	mutable std::mutex stateMutex;

	STI::Utils::DependencyTree<EngineState> stateTree;
};


} //Engine
} //STI

#endif
