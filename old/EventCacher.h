#ifndef STI_UTILS_EVENTCACHER_H
#define STI_UTILS_EVENTCACHER_H



namespace STI
{
namespace Utils
{


/*
Events added are queued.

States: Idle, Warming, Sending, Cooling

During Warming and Cooling, any new events are grouped

addMessage: Idle -> Warming
While Warming, new messages are grouped.
After warmup time: Warming -> Sending
Sending: Message group is converted and then sent
Sending -> Cooling.
While Cooling, new messages are grouped.
Cooling -> Idle. If queue is not empty, Idle->Warming

Warming period allows the Cacher to collect a burst of event that arrive at almost the same time
and send them at once.  It should be relatively short (<100 ms) so new messages are prompt.
Cooling period allows the Cacher to cap the maximum event rate (1 sec for example).
With this strategy, new isolated event bursts are prompt, but if too many events are sent
continuously, the maximum rate is limited.

Grouping considerations:
1) If events are "identical" they should conditionally overwrite, based on policy.

*/


template<class Event>
class EventCacher
{
public:

	enum class EventCacherState { Idle, Warming, Sending, Cooling };

    EventCacher() 
    : state(EventCacherState::Idle), running(false), messageCached(false) 
    {
        setWarmup(100);     //ms
        setCooldown(500);  //ms
    }
    ~EventCacher() { stop(); }

	void start();
	void stop();

    void addEvent(const std::shared_ptr<Event>& mess);

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


} // UTILS
} // STI


//Implementation


#endif




