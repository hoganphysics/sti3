#ifndef STI_ENGINE_EVENTCONFLICTEXCEPTION_H
#define STI_ENGINE_EVENTCONFLICTEXCEPTION_H

#include <sti/engine/RawEvent.h>
#include <sti/engine/STI_Exception.h>

#include <string>


namespace STI
{
namespace Engine
{

class EventConflictException : public STI_Exception
{
public:

	EventConflictException(const STI::Engine::RawEvent& evt, const std::string& message)
		: STI_Exception("Event Conflict Exception", message)
		{
			attachEvent(evt);
		}
	EventConflictException(const STI::Engine::RawEvent& event1, const STI::Engine::RawEvent& event2, const std::string& message)
		: STI_Exception("Event Conflict Exception", message)
		{
			attachEvent(event1);
			attachEvent(event2);
		}
	~EventConflictException() {}

	double lastTime() const
	{
		double last = 0;
		for (auto& evt : getEvents()) {
			if (evt.time() > last) {
				last = evt.time();
			}
		}
		return last;
	}
};


} //Engine
} //STI


#endif

