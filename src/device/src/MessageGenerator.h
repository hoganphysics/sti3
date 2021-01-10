
#ifndef STI_DEVICE_MESSAGEGENERATOR_H
#define STI_DEVICE_MESSAGEGENERATOR_H

#include "DeviceMessageDispatcher.h"

namespace STI
{
namespace Device
{


class MessageGenerator
{
public:

	MessageGenerator(const std::shared_ptr<DeviceMessageDispatcher>& dispatcher) : dispatcher(dispatcher) {}
	virtual ~MessageGenerator() {}

	template<typename T>
	void sendMessage(const std::shared_ptr<T>& message)
    {
        if(dispatcher != 0) {
            dispatcher->addMessage(message);
        }
    }

private:

    std::shared_ptr<DeviceMessageDispatcher> dispatcher;

};



} //Device
} //STI


#endif
