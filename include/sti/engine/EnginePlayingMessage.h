#ifndef STI_ENGINE_ENGINEPLAYINGMESSAGE_H
#define STI_ENGINE_ENGINEPLAYINGMESSAGE_H

#include <sti/utils/utils.h>
#include <sti/device/DeviceID.h>

#include <string>
#include <vector>


namespace STI
{
namespace Engine
{

enum class PlayingMessageType { Error, Warning, Information };


class EnginePlayingMessage
{
public:

    EnginePlayingMessage();
    EnginePlayingMessage(const PlayingMessageType& type, unsigned id, const std::string& name);
    EnginePlayingMessage(const STI::Device::DeviceID& source, 
                            const PlayingMessageType& type, unsigned id, const std::string& name);
    ~EnginePlayingMessage();

    void setSourceID(const STI::Device::DeviceID& source) { sourceID = source; }

    PlayingMessageType getType() const;
    unsigned getIDCode() const;
    STI::Device::DeviceID getSourceID() const;
    const std::string& getName() const;
    const std::string& getMessage() const;

    EnginePlayingMessage& appendMessage(const std::string& message);

	template<class T>
	EnginePlayingMessage& operator<< (const T& message)
	{
        return appendMessage(STI::Utils::valueToString(message));
	}

    template<class Archive>
    void serialize(Archive& archive);

private:

    STI::Device::DeviceID sourceID;
    PlayingMessageType type;
	unsigned id_code;
	std::string name;
	std::string message_;
};

} //Engine
} //STI

#endif
