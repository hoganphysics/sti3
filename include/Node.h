#ifndef STI_NETWORK_NODE_H
#define STI_NETWORK_NODE_H

#include "Collector.h"

namespace STI
{
namespace Network
{

template<class ID, class T>
class Node : public STI::Utils::Collector<ID, T>
{
public:
	virtual ~Node() {}
	//maybe node crawler hooks too?
	void ping() { return; }
	virtual bool refresh() = 0;
	//virtual T& get() = 0;
	T& get() { return static_cast<T&>(*this); }		//static polymorphism via CRTP

	T* operator->() { return &get(); }
};


} //Network
} //STI

#endif

