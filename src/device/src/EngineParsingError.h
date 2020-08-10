#ifndef STI_ENGINE_ENGINEPARSINGERROR_H
#define STI_ENGINE_ENGINEPARSINGERROR_H

#include "fwd/RawEvent_fwd.h"
#include "fwd/DeviceID_fwd.h"
#include "DeviceID.h"
#include "utils.h"

#include <sstream>
#include <string>
#include <vector>

namespace STI
{
namespace Engine
{

class EventConflictException;
class EventParsingException;

class EngineParsingError
{
public:
	EngineParsingError(const STI::Device::DeviceID& deviceID);
	EngineParsingError(const STI::Device::DeviceID& deviceID, const EventConflictException& exception);
	EngineParsingError(const STI::Device::DeviceID& deviceID, const EventParsingException& exception);

	~EngineParsingError();

	void addEvent(const RawEvent& evt);

	template<class T>
	EngineParsingError& operator<< (const T& message)
	{
		errMessage.append(STI::Utils::valueToString(message));
		return (*this);
	}

	std::string print() const;
	std::string messageText() const;

private:
	std::vector<RawEvent> events;
	std::string errMessage;

	unsigned errorcode;
	std::string name;	//short name of error
	const STI::Device::DeviceID& deviceID;

};

} //Engine
} //STI

#endif
