
#ifndef STI_DEVICE_MESSAGEGROUPER_H
#define STI_DEVICE_MESSAGEGROUPER_H


#include <sti/device/DeviceMessageDispatcher.h>

#include <sti/device/GroupableMessage.h>

#include <memory>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <chrono>


namespace STI
{
namespace Device
{



/*
Messages added are queued.

States: Idle, Warming, Sending, Cooling

During Warming and Cooling, any new messages are grouped

addMessage: Idle -> Warming
While Warming, new messages are grouped.
After warmup time: Warming -> Sending
Sending: Message group is converted and then sent
Sending -> Cooling.
While Cooling, new messages are grouped.
Cooling -> Idle. If queue is not empty, Idle->Warming

Warming period allows the Cacher to collect a burst of event that arrive at almost the same time
and send them at once.  It should be relatively short (<100 ms) so new messages are prompt.
Cooling period allows the Cacher to cap the maximum message rate (1 sec for example).
With this strategy, new isolated message bursts are prompt, but if too many messages are sent
continuously, the maximum rate is limited.

Grouping considerations:
1) If messages are "identical" they should conditionally overwrite, based on policy.



*/


// class ChannelUpdateDeviceMessage : public DeviceMessage, 
//                                    public GroupableMessage<ChannelUpdateDeviceMessage>
// {
// public:

// 	ChannelUpdateDeviceMessage(const STI::Device::DeviceID& source) 
//         : DeviceMessage(source, DeviceMessageType::ChannelUpdate) {}

//     std::map<short, MixedValue> channelValues;
    
//     bool appendMessage(const ChannelUpdateDeviceMessage& mess)
//     {
//         for (auto& pair : mess.channelValues) {
//             channelValues[pair->first] = pair->second;  //overwrite
//         }

// template<typename Message>
// class GroupableMessage
// {
// //    virtual bool appendMessage(const GroupableMessage<Message>& mess) = 0;

//     virtual bool appendMessage(const Message& mess) = 0;

//     virtual Message& get() = 0;
// };
// 	static DeviceMessageType getMessageClassType() { return DeviceMessageType::ChannelUpdate; }

// };

// class AbstractMessageGrouper
// {
// public:
// };


//States: Idle, Warming, Sending, Cooling

template<typename Message>
class MessageGrouper //MessageDelayer
{
public:

    enum class MessageGrouperState { Idle, Warming, Sending, Cooling };

    MessageGrouper(const std::shared_ptr<STI::Device::DeviceMessageDispatcher>& dispatcher) 
    : state(MessageGrouperState::Idle), messageDispatcher(dispatcher), running(false), messageCached(false) 
    {
        setWarmup(100);     //ms
        setCooldown(500);  //ms
    }
    ~MessageGrouper() { stop(); }

	void start();
	void stop();

    void addMessage(const std::shared_ptr<Message>& mess);

    void setWarmup(int time) { warmupTime = time; }
    void setCooldown(int time) { cooldownTime = time; }

private:

    void sendMessage();
    void appendMessage(const std::shared_ptr<GroupableMessage<Message>>& mess);

    void messageHandlerLoop();

    int warmupTime;     //ms
    int cooldownTime;   //ms

	std::thread eventThread;
	bool running;

    bool messageCached;
    MessageGrouperState state;
    std::shared_ptr<Message> message;

    std::shared_ptr<STI::Device::DeviceMessageDispatcher> messageDispatcher;

	mutable std::mutex cacherMutex;
	mutable std::condition_variable condition;

};

} // Device
} // STI



template<class Message>
void STI::Device::MessageGrouper<Message>::start()
{
	std::unique_lock<std::mutex> writeLock(cacherMutex);
	if (!running) {
		running = true;
		eventThread = std::thread(&MessageGrouper<Message>::messageHandlerLoop, this);
	}
}

template<class Message>
void STI::Device::MessageGrouper<Message>::stop()
{
	if (!running)
		return;

	{
		std::unique_lock<std::mutex> writeLock(cacherMutex);
		running = false;
		condition.notify_all();
	}

    if(eventThread.joinable()) {
        eventThread.join();
    }
}


template<class Message>
void STI::Device::MessageGrouper<Message>::addMessage(const std::shared_ptr<Message>& mess)
{
    std::unique_lock<std::mutex> writeLock(cacherMutex);

    if (!running || mess == 0) {
        return;
    }

    //For non-groupable messages, just send immediately
    if (!mess->groupable()) {
        if (messageDispatcher != 0) {
            messageDispatcher->addMessage(mess);            
        }
        return;
    }

    //For groupable messages
    switch (state)
    {
    case MessageGrouperState::Idle:
        message = mess;
        messageCached = true;
        state = MessageGrouperState::Warming;
        condition.notify_all();
        break;
    case MessageGrouperState::Warming:
    case MessageGrouperState::Cooling:
        if (messageCached) {
            appendMessage(mess);
        }
        else {
            message = mess;
            messageCached = true;
        }
        break;
    default:
        break;
    }
}

template<class Message>
void STI::Device::MessageGrouper<Message>::sendMessage()
{
    if (messageDispatcher != 0 && state == MessageGrouperState::Sending && messageCached) {
        messageDispatcher->addMessage(message);
        messageCached = false;
    }
}

template<class Message>
void STI::Device::MessageGrouper<Message>::appendMessage(const std::shared_ptr<GroupableMessage<Message>>& mess)
{
    if (mess != 0) {
        message->appendMessage(mess->get());
    }
}

template<class Message>
void STI::Device::MessageGrouper<Message>::messageHandlerLoop()
{
    while (running)
    {
       	std::unique_lock<std::mutex> writeLock(cacherMutex);
        
        //Idle
        while (state == MessageGrouperState::Idle && running) {
            condition.wait(writeLock);
        }

        //Warming
        if (state == MessageGrouperState::Warming && running) {
            condition.wait_for(writeLock, std::chrono::milliseconds(warmupTime));
        }

        //Sending
        if (state == MessageGrouperState::Warming && running) {
            state = MessageGrouperState::Sending;
            sendMessage();
            state = MessageGrouperState::Cooling;
        }
        
        //Cooling
        if (state == MessageGrouperState::Cooling && running) {
            condition.wait_for(writeLock, std::chrono::milliseconds(cooldownTime));
        }

        if (messageCached && running) {
            state = MessageGrouperState::Warming;
        }
        else {
            state = MessageGrouperState::Idle;
        }
    }
}


#endif

