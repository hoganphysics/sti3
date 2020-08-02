#include "utils/SynchronizedMap.h"
#include "utils/OrderedBufferMap.h"
#include <memory>

class A
{
public:
	int data;
};

STI::Utils::OrderedBufferMap<int, std::shared_ptr<A>> measurementBuffer(3);

STI::Utils::SynchronizedMap<int, std::shared_ptr<A>> temp;

class MyQueue : public STI::Utils::EventQueue<A>
{
public:
	void handleEvent(const A& evt) {}
};

MyQueue aqueue;

template<class Event>
class BufferedEventQueue : public STI::Utils::EventQueue<Event>
{
public:
	unsigned bufferSize;

	void handleEvent(const Event& evt) 
	{
		if (eventBuffer.size() < bufferSize) {
			eventBuffer.push_back(evt);
		}
	}

	std::vector<Event> eventBuffer;
};


