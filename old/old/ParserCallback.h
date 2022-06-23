#ifndef STI_ENGINE_PARSERCALLBACK_H
#define STI_ENGINE_PARSERCALLBACK_H

#include <sti/device/DeviceID.h>
#include <sti/engine/EngineID.h>

#include <sti/fwd/RawEvent_fwd.h>

#include <string>
#include <memory>

namespace STI
{
namespace Engine
{

class ParserCallbackMessage;
class ParserCallbackTarget;

class ParserCallbackTarget		//interface
{
public:

	virtual ~ParserCallbackTarget() {}

	virtual void handleParsingResults(const ParserCallbackMessage& message) = 0;
};

class ParserCallback
{
public:

	typedef std::shared_ptr<ParserCallback> _ptr;

	ParserCallback(ParserCallbackTarget* target) : _target(target) {}
	~ParserCallback() {}

	void returnResults(const ParserCallbackMessage& message) { _target->handleParsingResults(message); }

private:

	ParserCallbackTarget* _target;
};

class ParserCallbackMessage
{
public:
	STI::Device::DeviceID deviceID;
	STI::Engine::EngineID engineID;
	bool success;
	std::string errors;
	STI::Engine::DeviceEventMap_ptr eventsOut;
};





} //Engine
} //STI

#endif
