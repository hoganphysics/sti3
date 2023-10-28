#ifndef STI_ENGINE_STI_EXCEPTION_H
#define STI_ENGINE_STI_EXCEPTION_H

#include <sti/engine/RawEvent.h>

#include <string>
#include <exception>
#include <vector>


namespace STI
{
namespace Engine
{

class STI_Exception : public std::exception
{
public:

	STI_Exception(const std::string& message) : name("Parsing Exception"), message(message) {};
	STI_Exception(const std::string& name, const std::string& message) : name(name), message(message) {};
	virtual ~STI_Exception() {}

	std::string getName() const { return name; }
	const std::vector<RawEvent>& getEvents() const { return events; }
	void attachEvent(const RawEvent& evt) { events.push_back(evt); }

	std::string printMessage() const { return message; }

private:

	std::string name;
	std::vector<RawEvent> events;
	std::string message;
};

} //Engine
} //STI


#endif
