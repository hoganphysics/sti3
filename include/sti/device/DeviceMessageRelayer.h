#ifndef STI_DEVICE_DEVICEMESSAGERELAYER_H
#define STI_DEVICE_DEVICEMESSAGERELAYER_H


#include <sti/device/DeviceMessageListener.h>
#include <sti/device/DeviceMessageReceiver.h>
#include <sti/device/DeviceMessageDispatcher.h>
#include <sti/device/DeviceID.h>

#include <memory>
#include <mutex>

namespace STI
{
namespace Device
{

class DeviceMessageDispatcher;

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

    template<typename W>
    void addFilter(const std::function<bool(const std::shared_ptr<W>&)>& filter) 
    {
        DeviceMessageRelayer<U>::addFilter(filter);
        DeviceMessageRelayer<T ...>::addFilter(filter);
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

    void addFilter(const std::function<bool(const std::shared_ptr<M>&)>& filter) 
    {
        //match
        std::lock_guard<std::mutex> lock(filtersMutex);
        DeviceMessageRelayer<M>::filters.push_back(filter);
    }

    template<typename U>
    void addFilter(const std::function<bool(const std::shared_ptr<U>&)>& filter) 
    {
        //catch wrong type
    }

	void handleMessage(const std::shared_ptr<M>& message) 
    {
        if (message == 0) return;
        
        if (message->getDeviceTrace().includesID(DeviceMessageRelayer<M>::relayerID)) {
            return; //loop detected
        }

        {
            std::lock_guard<std::mutex> lock(filtersMutex);

            //apply filters
            for (auto& relayQ : DeviceMessageRelayer<M>::filters) {
                if(!relayQ(message)) return;
            }
        }

        //relay message
        message->addRelayingID(DeviceMessageRelayer<M>::relayerID);

        if (DeviceMessageRelayer<M>::dispatcher != 0) {
            DeviceMessageRelayer<M>::dispatcher->addMessage(message);
        }
    }

    static void addAllListeners(const std::shared_ptr<DeviceMessageReceiver>& receiver, 
                                const DeviceID& id, const std::shared_ptr<DeviceMessageRelayer<M>>& relayer)
    {
        auto listener = std::static_pointer_cast<DeviceMessageListener<M>>(relayer);


        if (receiver != 0) {
	        receiver->addListener(id, relayer->DeviceMessageRelayer<M>::listenerID, listener);
        }
    }

    static void removeAllListeners(const std::shared_ptr<DeviceMessageReceiver>& receiver, 
                                    const DeviceID& id, const std::shared_ptr<DeviceMessageRelayer<M>>& relayer)
    {
        if (receiver != 0) {
    	    receiver->removeListener(id, relayer->DeviceMessageRelayer<M>::listenerID);
        }
    }

private:

    std::vector<std::function<bool(const std::shared_ptr<M>&)>> filters;
    std::mutex filtersMutex;

    DeviceID relayerID;
    DeviceMessageListenerID listenerID;
    std::shared_ptr<DeviceMessageDispatcher> dispatcher;

};


} //Device
} //STI

#endif

