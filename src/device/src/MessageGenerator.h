
#ifndef STI_DEVICE_MESSAGEGENERATOR_H
#define STI_DEVICE_MESSAGEGENERATOR_H

#include "DeviceEventDispatcher.h"

namespace STI
{
namespace Device
{


class MessageGenerator
{
public:

	MessageGenerator(const std::shared_ptr<DeviceEventDispatcher>& dispatcher) : dispatcher(dispatcher) {}
	~MessageGenerator() {}

	template<typename T>
	void sendMessage(const std::shared_ptr<T>& message)
    {
        if(dispatcher != 0) {
            dispatcher->addEvent(message);
        }
    }

private:

    std::shared_ptr<DeviceEventDispatcher> dispatcher;

};



} //Device
} //STI


#endif
