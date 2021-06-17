
#include "EngineParsingMessage.h"
#include "RawEvent.h"

using STI::Engine::EngineParsingMessage;
using STI::Engine::ParsingMessageType;
using STI::Engine::RawEvent;


EngineParsingMessage::EngineParsingMessage(const STI::Device::DeviceID& source, 
                                            const ParsingMessageType& type, unsigned id, const std::string& name)
: sourceID(source), type(type), id_code(id), name(name)
{   
}

EngineParsingMessage::~EngineParsingMessage()
{
}

ParsingMessageType EngineParsingMessage::getType() const
{
    return type;
}

STI::Device::DeviceID EngineParsingMessage::getSourceID() const
{
    return sourceID;
}

unsigned EngineParsingMessage::getIDCode() const
{
    return id_code;
}

const std::string& EngineParsingMessage::getName() const
{
    return name;
}

const std::string& EngineParsingMessage::getMessage() const
{
    return message_;
}

const std::vector<RawEvent>& EngineParsingMessage::getEvents() const
{
    return events;
}


EngineParsingMessage& EngineParsingMessage::addEvent(const RawEvent& evt)
{
    events.push_back(evt);
    return (*this);
}

EngineParsingMessage& EngineParsingMessage::appendMessage(const std::string& message)
{
    message_.append(message);
    return (*this);
}

void EngineParsingMessage::setEvents(std::vector<RawEvent>& evts)
{
    events.clear();
    events = evts;
}

