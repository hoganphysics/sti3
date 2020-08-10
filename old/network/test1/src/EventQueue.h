#ifndef STI_UTILS_EVENTQUEUE_H
#define STI_UTILS_EVENTQUEUE_H

#include <memory>
#include <mutex>
#include <thread>
#include <deque>

namespace STI
{
namespace Utils
{


template<class Event>
class EventQueue
{
public:
	EventQueue();
	~EventQueue();

	//Adds event to the back of the event queue (FIFO behavior)
	void addEvent(const Event& evt);

	//Adds event to the front of the event queue (for high priority events)
	void addPriorityEvent(const Event& evt);

	void clearEvents();

	void start();
	void stop();

private:

	//Interface
	virtual void handleEvent(const Event& evt) = 0;
	virtual bool readyToHandleEvent(const Event& evt) { return true; }

	bool _isNextEventReady();
	void wait();
	void eventHandlerLoop();

	std::deque<Event> fifo;
	std::thread eventThread;
	bool running;

	mutable std::mutex fifoMutex;
	mutable std::condition_variable condition;
};


} // UTILS
} // STI


//Implementation

template<class Event>
STI::Utils::EventQueue<Event>::EventQueue() : running(false)
{
}

template<class Event>
STI::Utils::EventQueue<Event>::~EventQueue() 
{
	stop();
}

template<class Event>
void STI::Utils::EventQueue<Event>::addEvent(const Event& evt)
{
	std::unique_lock<std::mutex> writeLock(fifoMutex);
	if (running) {
		fifo.push_back(evt);
		condition.notify_all();
	}
}

template<class Event>
void STI::Utils::EventQueue<Event>::addPriorityEvent(const Event& evt)
{
	std::unique_lock<std::mutex> writeLock(fifoMutex);
	if (running) {
		fifo.push_front(evt);
		condition.notify_all();
	}
}

template<class Event>
void STI::Utils::EventQueue<Event>::clearEvents()
{
	std::unique_lock<std::mutex> writeLock(fifoMutex);
	fifo.clear();
	condition.notify_all();
}

template<class Event>
void STI::Utils::EventQueue<Event>::start()
{
	std::unique_lock<std::mutex> writeLock(fifoMutex);
	if (!running) {
		running = true;
		eventThread = std::thread(&EventQueue<Event>::eventHandlerLoop, this);
	}
}

template<class Event>
void STI::Utils::EventQueue<Event>::stop()
{
	if (!running)
		return;

	{
		std::unique_lock<std::mutex> writeLock(fifoMutex);
		running = false;
		condition.notify_all();
	}
	eventThread.join();
}

template<class Event>
void STI::Utils::EventQueue<Event>::wait()
{
	std::unique_lock<std::mutex> writeLock(fifoMutex);

	while (!_isNextEventReady() && running) {
		condition.wait(writeLock);
	}
}

template<class Event>
bool STI::Utils::EventQueue<Event>::_isNextEventReady()
{
	if (!fifo.empty()) {
		return readyToHandleEvent(fifo.front());
	}
	return false;
}

template<class Event>
void STI::Utils::EventQueue<Event>::eventHandlerLoop()
{
	Event eventToHandle;

	while (running)
	{
		wait();

		{
			std::unique_lock<std::mutex> writeLock(fifoMutex);

			if (!fifo.empty()) {	//just in case, check for events
				eventToHandle = fifo.front();
				fifo.pop_front();
			}
		}

		if (running) {		//check for recent call to stop()
			handleEvent(eventToHandle);
		}
	}
}


#endif

