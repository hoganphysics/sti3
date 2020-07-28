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
	EventQueue() : running(false) { }
	~EventQueue() { stop(); }

	void handleEvents()
	{
		std::unique_lock<std::mutex> writeLock(fifoMutex);
		condition.notify_all();
	}

	//Adds event to the back of the event queue (FIFO behavior)
	void addEvent(const Event& evt)
	{
		std::unique_lock<std::mutex> writeLock(fifoMutex);
		if (running) {
			fifo.push_back(evt);
			condition.notify_all();
		}
	}

	//Adds event to the front of the event queue (for high priority events)
	void addPriorityEvent(const Event& evt)
	{
		std::unique_lock<std::mutex> writeLock(fifoMutex);
		if (running) {
			fifo.push_front(evt);
//			fifoEmpty = fifo.empty();
			condition.notify_all();
		}
	}

	void clearEvents()
	{
		std::unique_lock<std::mutex> writeLock(fifoMutex);
		fifo.clear();
		condition.notify_all();
	}

	void start()
	{
		std::unique_lock<std::mutex> writeLock(fifoMutex);
		if (!running) {
			running = true;
			eventThread = std::thread(&EventQueue<Event>::eventHandlerLoop, this);
		}
	}

	void stop()
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

private:

	virtual void handleEvent(const Event& evt) = 0;
	virtual bool readyToHandleEvent(const Event& evt) { return true; }

	void wait()
	{
		std::unique_lock<std::mutex> writeLock(fifoMutex);
//		bool nextEventReady = false;

		while (!_isNextEventReady() && running) {
			condition.wait(writeLock);
//			fifoEmpty = fifo.empty();
		}
	}
	
	bool _isNextEventReady()
	{
		if (!fifo.empty()) {
			return readyToHandleEvent(fifo.front());
		}
		return false;
	}

	void eventHandlerLoop()
	{
		Event eventToHandle;
//		bool handleEventNow = false;

		while (running)
		{
			wait();
			
			{
				std::unique_lock<std::mutex> writeLock(fifoMutex);
				
				if (!fifo.empty()) {	//just in case, check for events
					eventToHandle = fifo.front();
					fifo.pop_front();
//					fifoEmpty = fifo.empty();
//					handleEventNow = true;
				}

				//handleEventNow = false;

				//if (!fifo.empty()) {	//just in case, check for events
				//	eventToHandle = fifo.front();
				//	handleEventNow = readyToHandleEvent(eventToHandle);
				//}
				//if (handleEventNow) {
				//	fifo.pop_front();
				//	fifoEmpty = fifo.empty();
				//}

			}

			if (running) {		//check for recent call to stop()
				handleEvent(eventToHandle);
			}
		}
	}

	std::deque<Event> fifo;
//	bool fifoEmpty;
	bool running;
//	bool eventsReady;

	std::thread eventThread;
	mutable std::mutex fifoMutex;
	mutable std::condition_variable condition;
};


} // UTILS
} // STI


#endif

