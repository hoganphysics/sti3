#ifndef STI_UTILS_NODE_H
#define STI_UTILS_NODE_H

#include <sti/utils/Collector.h>

namespace STI
{
namespace Utils
{

template<class ID, class T>
class Node : public STI::Utils::Collector<ID, T>
{
public:
	//maybe node crawler hooks too?
	void ping() { return; }
	virtual bool refresh() = 0;
	//virtual T& get() = 0;
	T& get() { return static_cast<T&>(*this); }		//static polymorphism via CRTP

	T* operator->() { return &get(); }
};


} //Utils
} //STI

#endif

