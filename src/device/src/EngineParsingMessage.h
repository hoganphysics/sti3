#ifndef STI_ENGINE_ENGINEPARSINGMESSAGE_H
#define STI_ENGINE_ENGINEPARSINGMESSAGE_H

#include "utils.h"

#include <string>
#include <vector>

namespace STI
{
namespace Engine
{

class RawEvent;

enum class ParsingMessageType { Error, Warning, Information };

class EngineParsingMessage
{
public:

    EngineParsingMessage(const ParsingMessageType& type, unsigned id, const std::string& name);
    ~EngineParsingMessage();

    ParsingMessageType getType() const;
    unsigned getID() const;
    const std::string& getName() const;
    const std::string& getMessage() const;
    const std::vector<RawEvent>& getEvents() const;

	EngineParsingMessage& addEvent(const RawEvent& evt);
    EngineParsingMessage& appendMessage(const std::string& message);

	template<class T>
	EngineParsingMessage& operator<< (const T& message)
	{
        return appendMessage(STI::Utils::valueToString(message));
	}

private:

    ParsingMessageType type;
	unsigned id_code;
	std::string name;
	std::string message_;
	std::vector<RawEvent> events;

};

} //Engine
} //STI

#endif
