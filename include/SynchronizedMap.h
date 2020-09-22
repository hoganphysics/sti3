#ifndef STI_UTILS_SYNCHRONIZEDMAP_H
#define STI_UTILS_SYNCHRONIZEDMAP_H

/*
SynchronizedMap: A template container for policy-based storage of object (typically references).

The class is Synchronized, which means that all calls are mutex-locked for thread safty.
It's a Map, which means it's an associative container of (Key, Value) pairs.
The map is policy-based.  This means that new elements will be added only if their Keys are
allowed by the custom include policy of the SynchronizedMap.

The class also generates Events when items are added or removed from the map.  It accepts listeners for these events and 
pushs events.

To do:  Refactor SynchronizedMap so there is a base class with no Event pusher, and derive a 

EmittingSynchronizedMap
EESynchronizedMap
EventEmittingSynchronizedMap
ActiveSynchronizedMap
EventDispatcherSynchronizedMap

*/

#include "EventQueue.h"

#include <vector>
#include <memory>
#include <mutex>
#include <map>
#include <set>

namespace STI
{
namespace Utils
{

template<class Key>
class SynchronizedMapListener
{
public:
	typedef std::shared_ptr<SynchronizedMapListener<Key>> _ptr;

	virtual void add(const Key& key) = 0;
	virtual void remove(const Key& key) = 0;
	virtual void refresh() = 0;
};

template<class Key>
class SynchronizedMapEvent
{
public:
	enum Type {Add, Remove, Refresh};
	SynchronizedMapEvent(const Key& key, const Type& type) : key(key), type(type) {}
	SynchronizedMapEvent(const Type& type) : type(type) {}
	SynchronizedMapEvent() {}

	Key key;
	Type type;
};

template<class Key>
class SynchronizedMapEventHandler : public EventQueue<SynchronizedMapEvent<Key>>
{
public:
	typedef std::vector<typename SynchronizedMapListener<Key>::_ptr> ListenerVector;

	void addListener(const typename SynchronizedMapListener<Key>::_ptr& listener)
	{
		STI::Utils::EventQueue<SynchronizedMapEvent<Key>>::start();	//start event handler loop (does nothing if already running)
		listeners.push_back(listener);
	}

private:
	void handleEvent(const SynchronizedMapEvent<Key>& evt)
	{
		for (typename ListenerVector::iterator it = listeners.begin(); it != listeners.end(); ++it) {
			switch (evt.type) {
			case SynchronizedMapEvent<Key>::Type::Add:
				(*it)->add(evt.key);
				break;
			case SynchronizedMapEvent<Key>::Type::Remove:
				(*it)->remove(evt.key);
				break;
			case SynchronizedMapEvent<Key>::Type::Refresh:
				(*it)->refresh();
				break;
			}
		}
	}

	ListenerVector listeners;
};

template<class Key>
class SynchronizedMapPolicy
{
public:
	virtual ~SynchronizedMapPolicy() {}

	virtual bool include(const Key& key) const = 0;
	virtual bool replace(const Key& oldKey, const Key& newKey) const = 0;
};

template<class Key>
class DefaultSynchronizedMapPolicy : public SynchronizedMapPolicy<Key>
{
	bool include(const Key& key) const { return true; }
	bool replace(const Key& oldKey, const Key& newKey) const { return (oldKey == newKey); }
};


//Const Key is not allowed because std::set<Key> cannot have const Key (must be copyable in STL)

template<class Key, class T>
class SynchronizedMap
{
public:

	typedef std::map<Key, T> TMap;
	typedef SynchronizedMapPolicy<Key> KeyPolicy;
	typedef std::shared_ptr<KeyPolicy> KeyPolicy_ptr;

	SynchronizedMap();
	//{
	//	typedef DefaultSynchronizedMapPolicy<Key> DefPol;

	//	std::shared_ptr<DefPol> defaultPolicy = boost::make_shared<DefPol>();
	//	setPolicy(defaultPolicy);
	//}
//	SynchronizedMap(KeyPolicy_ptr Policy) : policy(defaultPolicy) { setPolicy(Policy); }
	SynchronizedMap(KeyPolicy_ptr Policy);	// : SynchronizedMap() { setPolicy(Policy); }
	virtual ~SynchronizedMap(); // { }

	void setPolicy(KeyPolicy_ptr Policy);
	void addListener(const typename SynchronizedMapListener<Key>::_ptr& listener);

	bool contains(const Key& key) const;
	bool include(const Key& key) const;
	bool get(const Key& key, T& item) const;
	void getKeys(std::set<Key>& keys) const;
	unsigned size() const;

	bool add(const Key& key, T item);
	bool remove(const Key& key);
	
	void cleanup();
	void clear();

private:

	//Internal functions with no mutex protection.
	bool _contains(const Key& key) const;
	bool _include(const Key& key) const;

//	static shared_ptr<KeyPolicy> defaultPolicy;
	
	
//	SynchronizedMapEventHandler< SynchronizedMapEvent<Key> > eventHandler;
	SynchronizedMapEventHandler< Key > eventHandler;
	void pushAddEvent(const Key& key);
	void pushRemoveEvent(const Key& key);
	void pushRefreshEvent();


	KeyPolicy_ptr policy;
	TMap items;

	mutable std::mutex mapMutex;
};

//template<class Key, class T>
//shared_ptr<DefaultSynchronizedMapPolicy<Key> > SynchronizedMap<Key, T>::defaultPolicy = shared_ptr<DefaultSynchronizedMapPolicy<Key> >(new DefaultSynchronizedMapPolicy<Key>());

} // UTILS
} // STI


//Implementation

template<class Key, class T>
STI::Utils::SynchronizedMap<Key, T>::SynchronizedMap()
{
	typedef DefaultSynchronizedMapPolicy<Key> DefPol;

	std::shared_ptr<DefPol> defaultPolicy = std::make_shared<DefPol>();
	setPolicy(defaultPolicy);
}

template<class Key, class T>
STI::Utils::SynchronizedMap<Key, T>::SynchronizedMap(KeyPolicy_ptr Policy) 
{ 
	setPolicy(Policy);
}

template<class Key, class T>
STI::Utils::SynchronizedMap<Key, T>::~SynchronizedMap()
{
}

template<class Key, class T>
void STI::Utils::SynchronizedMap<Key, T>::setPolicy(KeyPolicy_ptr Policy)
{
	std::unique_lock< std::mutex > writeLock(mapMutex);
	if (Policy != 0)
		policy = Policy;
}

template<class Key, class T>
void STI::Utils::SynchronizedMap<Key, T>::addListener(const typename SynchronizedMapListener<Key>::_ptr& listener)
{
	eventHandler.addListener(listener);
	eventHandler.start();
}

template<class Key, class T>
void STI::Utils::SynchronizedMap<Key, T>::pushAddEvent(const Key& key)
{
	eventHandler.addEvent(SynchronizedMapEvent<Key>(key, SynchronizedMapEvent<Key>::Type::Add));
}

template<class Key, class T>
void STI::Utils::SynchronizedMap<Key, T>::pushRemoveEvent(const Key& key)
{
	eventHandler.addEvent(SynchronizedMapEvent<Key>(key, SynchronizedMapEvent<Key>::Type::Remove));
}

template<class Key, class T>
void STI::Utils::SynchronizedMap<Key, T>::pushRefreshEvent()
{
	eventHandler.addEvent(SynchronizedMapEvent<Key>(SynchronizedMapEvent<Key>::Type::Refresh));
}

template<class Key, class T>
bool STI::Utils::SynchronizedMap<Key, T>::contains(const Key& key) const
{
	std::unique_lock< std::mutex > readLock(mapMutex);
	return _contains(key);
}

template<class Key, class T>
bool STI::Utils::SynchronizedMap<Key, T>::_contains(const Key& key) const
{
	return (items.find(key) != items.end());
}

template<class Key, class T>
bool STI::Utils::SynchronizedMap<Key, T>::include(const Key& key) const
{
	std::unique_lock< std::mutex > readLock(mapMutex);
	if(policy != 0)
		return _include(key);
	return false;
}

template<class Key, class T>
bool STI::Utils::SynchronizedMap<Key, T>::_include(const Key& key) const
{
	return policy->include(key);
}

template<class Key, class T>
bool STI::Utils::SynchronizedMap<Key, T>::add(const Key& key, T item)
{
	std::unique_lock< std::mutex > writeLock(mapMutex);

	if (policy == 0)
		return false;

	if (policy->include(key)) {
		if (_contains(key) && policy->replace(items.find(key)->first, key)) {
			items.erase(key);
			items.insert(std::make_pair(key, item));
		}
		else {
			items.insert(std::make_pair(key, item));
		}
		pushAddEvent(key);
		return _contains(key);
	}
	return false;
}

template<class Key, class T>
bool STI::Utils::SynchronizedMap<Key, T>::remove(const Key& key)
{
	std::unique_lock< std::mutex > writeLock(mapMutex);
	items.erase(key);
	pushRemoveEvent(key);
	return !_contains(key);
}

template<class Key, class T>
bool STI::Utils::SynchronizedMap<Key, T>::get(const Key& key, T& item) const
{
	std::unique_lock< std::mutex > readLock(mapMutex);

	typename TMap::const_iterator it = items.find(key);

	if(it != items.end())
	{
		item = it->second; 
		return true;
	}
	return false;
}

template<class Key, class T>
void STI::Utils::SynchronizedMap<Key, T>::getKeys(std::set<Key>& keys) const
{
	keys.clear();

	std::unique_lock< std::mutex > readLock(mapMutex);

	for(typename TMap::const_iterator it = items.begin(); it != items.end(); ++it)
	{
		keys.insert(it->first);
	}
}

template<class Key, class T>
unsigned STI::Utils::SynchronizedMap<Key, T>::size() const
{
	std::unique_lock< std::mutex > readLock(mapMutex);
	return static_cast<unsigned>(items.size());
}

template<class Key, class T>
void STI::Utils::SynchronizedMap<Key, T>::cleanup()
{
	//Remove any items that do not satisfy the policy.

	std::unique_lock< std::mutex > writeLock(mapMutex);

	typename TMap::const_iterator it = items.begin();
	
	while(it != items.end())
	{
		if(!_include(it->first)) {
			pushRemoveEvent(it->first);
			items.erase(it++);	//post increment so it returns original 'it' after incrementing
		}
		else {
			++it;
		}
	}
}

template<class Key, class T>
void STI::Utils::SynchronizedMap<Key, T>::clear()
{
	std::unique_lock< std::mutex > writeLock(mapMutex);
	items.clear();
	pushRefreshEvent();
}


#endif

