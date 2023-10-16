#ifndef STI_ENGINE_EVENTPARSINGEXCEPTION_H
#define STI_ENGINE_EVENTPARSINGEXCEPTION_H

#include <sti/engine/RawEvent.h>
#include <sti/engine/STI_Exception.h>

#include <string>


namespace STI
{
namespace Engine
{


class EventParsingException : public STI_Exception
{
public:

	EventParsingException(const STI::Engine::RawEvent& evt, const std::string& message)
		: STI_Exception(message), _evt(evt) {}
	~EventParsingException() throw() {}

	const STI::Engine::RawEvent& getEvent() const { return _evt; };

private:

	const STI::Engine::RawEvent _evt;
};


} //Engine
} //STI


#endif
