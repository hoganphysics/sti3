
#include <sti/engine/EnginePlayingMessage.h>

#include "CerealArchives.h"
#include <cereal/types/map.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/vector.hpp>

using STI::Engine::EnginePlayingMessage;
using STI::Engine::PlayingMessageType;


EnginePlayingMessage::EnginePlayingMessage()
{
}

EnginePlayingMessage::EnginePlayingMessage(const PlayingMessageType& type, unsigned id, const std::string& name)
: type(type), id_code(id), name(name)
{
}

EnginePlayingMessage::EnginePlayingMessage(const STI::Device::DeviceID& source, 
                                            const PlayingMessageType& type, unsigned id, const std::string& name)
: sourceID(source), type(type), id_code(id), name(name)
{   
}

EnginePlayingMessage::~EnginePlayingMessage()
{
}

PlayingMessageType EnginePlayingMessage::getType() const
{
    return type;
}

STI::Device::DeviceID EnginePlayingMessage::getSourceID() const
{
    return sourceID;
}

unsigned EnginePlayingMessage::getIDCode() const
{
    return id_code;
}

const std::string& EnginePlayingMessage::getName() const
{
    return name;
}

const std::string& EnginePlayingMessage::getMessage() const
{
    return message_;
}


EnginePlayingMessage& EnginePlayingMessage::appendMessage(const std::string& message)
{
    message_.append(message);
    return (*this);
}


template<class Archive>
void EnginePlayingMessage::serialize(Archive& archive)
{
    archive( 
        cereal::make_nvp("sourceID", sourceID),
        cereal::make_nvp("type", type),
        cereal::make_nvp("id_code", id_code),
        cereal::make_nvp("name", name),
        cereal::make_nvp("message", message_)
        );
}

template void EnginePlayingMessage::serialize<cereal::XMLOutputArchive>( cereal::XMLOutputArchive& );
template void EnginePlayingMessage::serialize<cereal::XMLInputArchive>( cereal::XMLInputArchive& );

