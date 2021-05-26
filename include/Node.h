#ifndef STI_NETWORK_NODE_H
#define STI_NETWORK_NODE_H

#include "Collector.h"

namespace STI
{
namespace Network
{

class HubID;

template<class ID, class T>
class Node : public STI::Utils::Collector<ID, T>
{
public:

	virtual ~Node() {}

	void ping() { return; }

	virtual bool refresh() = 0;
	virtual void disable() = 0;

	//Optimization to allow Nodes to be selective about when Hubs they are broadcast to.
	//If the Node is known to be restricted to specific hubs, this potentially avoid unneeded network calls.
	virtual bool addto(const HubID& target) = 0;

	//virtual T& get() = 0;
	T& get() { return static_cast<T&>(*this); }		//static polymorphism via CRTP
	T* operator->() { return &get(); }
};


} //Network
} //STI

#endif

