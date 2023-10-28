#ifndef STI_ENGINE_EVENTPARSINGEXCEPTION_H
#define STI_ENGINE_EVENTPARSINGEXCEPTION_H

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
		: STI_Exception("Event Parsing Exception", message) { attachEvent(evt); }
	~EventParsingException() {}

private:

};


} //Engine
} //STI


#endif
