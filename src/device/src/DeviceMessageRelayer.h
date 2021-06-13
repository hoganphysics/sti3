#ifndef STI_DEVICE_DEVICEMESSAGERELAYER_H
#define STI_DEVICE_DEVICEMESSAGERELAYER_H


#include "DeviceMessageListener.h"
#include "DeviceMessageReceiver.h"
#include "DeviceMessageDispatcher.h"
#include "DeviceID.h"

#include <memory>

#include <iostream>


namespace STI
{
namespace Device
{

//Recursive variadic template is used so DeviceMessageRelayer<U, T ...> can inherit from listeners of any set of message types.
//Recursion terminates when it reaches specialization DeviceMessageRelayer<M>, defined below.

template<class U, class... T>
class DeviceMessageRelayer : public DeviceMessageRelayer<U>, public DeviceMessageRelayer<T ...>
{
public:

    DeviceMessageRelayer(const DeviceID& relayerID, const std::shared_ptr<DeviceMessageDispatcher>& dispatcher) 
    : DeviceMessageRelayer<U>(relayerID, dispatcher), DeviceMessageRelayer<T ...>(relayerID, dispatcher) {}

    virtual ~DeviceMessageRelayer() {}

    static void addAllListeners(const std::shared_ptr<DeviceMessageReceiver>& receiver, 
                                const DeviceID& id, const std::shared_ptr<DeviceMessageRelayer<U, T ...>>& relayer)
    {
        DeviceMessageRelayer<U>::addAllListeners(receiver, id, relayer);
        DeviceMessageRelayer<T ...>::addAllListeners(receiver, id, relayer);
    }

    static void removeAllListeners(const std::shared_ptr<DeviceMessageReceiver>& receiver, 
                                   const DeviceID& id, const std::shared_ptr<DeviceMessageRelayer<U, T ...>>& relayer)
    {
        DeviceMessageRelayer<U>::removeAllListeners(receiver, id, relayer);
        DeviceMessageRelayer<T ...>::removeAllListeners(receiver, id, relayer);
    }

};


//Specialization.  All message types generate a mixin of this type.

template<class M>
class DeviceMessageRelayer<M> : public DeviceMessageListener<M>
{
public:
    
    DeviceMessageRelayer(const DeviceID& relayerID, const std::shared_ptr<DeviceMessageDispatcher>& dispatcher) 
    : relayerID(relayerID), dispatcher(dispatcher)
    {
	    DeviceMessageRelayer<M>::listenerID.type = M::getMessageClassType();
        DeviceMessageRelayer<M>::listenerID.name = DeviceMessageRelayer<M>::relayerID.getID() + "::Relayer::" + M::typeToString( M::getMessageClassType() );
    }

    virtual ~DeviceMessageRelayer() {}

	void handleMessage(const std::shared_ptr<M>& mess) 
    {
        if (mess == 0) return;
        
        if (mess->getDeviceTrace().includesID(DeviceMessageRelayer<M>::relayerID)) {
            return; //loop detected
        }

        //relay message
        mess->addRelayingID(DeviceMessageRelayer<M>::relayerID);

        if (DeviceMessageRelayer<M>::dispatcher != 0) {
            DeviceMessageRelayer<M>::dispatcher->addMessage(mess);
        }
    }

    static void addAllListeners(const std::shared_ptr<DeviceMessageReceiver>& receiver, 
                                const DeviceID& id, const std::shared_ptr<DeviceMessageRelayer<M>>& relayer)
    {
        auto listener = std::static_pointer_cast<DeviceMessageListener<M>>(relayer);


        if (receiver != 0) {
	        receiver->addListener(id, relayer->DeviceMessageRelayer<M>::listenerID, listener);
            // std::cout << "add DeviceMessageRelayer<" << M::typeToString( M::getMessageClassType() ) << ">" << std::endl;
        }
    }

     static void removeAllListeners(const std::shared_ptr<DeviceMessageReceiver>& receiver, 
                                    const DeviceID& id, const std::shared_ptr<DeviceMessageRelayer<M>>& relayer)
    {
        // std::cout << "remove DeviceMessageRelayer<" << M::typeToString( M::getMessageClassType() ) << ">" << std::endl;
        if (receiver != 0) {
    	    receiver->removeListener(id, relayer->DeviceMessageRelayer<M>::listenerID);
            // std::cout << "remove DeviceMessageRelayer<" << M::typeToString( M::getMessageClassType() ) << ">" << std::endl;
        }
    }

private:

    DeviceID relayerID;
    DeviceMessageListenerID listenerID;
    std::shared_ptr<DeviceMessageDispatcher> dispatcher;

};


} //Device
} //STI

#endif

