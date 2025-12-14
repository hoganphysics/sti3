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

*/

#include <sti/utils/EventQueue.h>

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

	virtual ~SynchronizedMapListener() {}

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

	virtual ~SynchronizedMapEvent() {}

	Key key;
	Type type;
};

template<class Key>
class SynchronizedMapEventHandler : public EventQueue<SynchronizedMapEvent<Key>>
{
public:
	typedef std::vector<typename SynchronizedMapListener<Key>::_ptr> ListenerVector;

	virtual ~SynchronizedMapEventHandler()
	{
		EventQueue<SynchronizedMapEvent<Key>>::stop();
	}

	void addListener(const typename SynchronizedMapListener<Key>::_ptr& listener)
	{
		std::unique_lock<std::mutex> writeLock(listenersMutex);

		STI::Utils::EventQueue<SynchronizedMapEvent<Key>>::start();	//start event handler loop (does nothing if already running)
		listeners.push_back(listener);
	}
	void clearListeners()
	{
		std::unique_lock<std::mutex> writeLock(listenersMutex);
		listeners.clear();
	}

private:
	void handleEvent(const SynchronizedMapEvent<Key>& evt)
	{
		std::unique_lock<std::mutex> writeLock(listenersMutex);

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
	mutable std::mutex listenersMutex;
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
	SynchronizedMap(const KeyPolicy_ptr& Policy);
	virtual ~SynchronizedMap(); // { }

	void setPolicy(const KeyPolicy_ptr& Policy);
	void addListener(const typename SynchronizedMapListener<Key>::_ptr& listener);
	void clearListeners();

	bool contains(const Key& key) const;
	bool include(const Key& key) const;
	bool get(const Key& key, T& item) const;
	void getKeys(std::set<Key>& keys) const;
	void getValues(std::vector<T>& values) const;
	void getValues(const std::set<Key>& keys, std::vector<T>& values) const;
	unsigned size() const;

	bool add(const Key& key, T item);
	bool remove(const Key& key);
	
	void cleanup();
	void clear();

private:

	//Internal functions with no mutex protection.
	bool _contains(const Key& key) const;
	bool _include(const Key& key) const;
	void _getKeys(std::set<Key>& keys) const;
	bool _get(const Key& key, T& item) const;
	bool _remove(const Key& key);

	SynchronizedMapEventHandler<Key> eventHandler;
	void pushAddEvent(const Key& key);
	void pushRemoveEvent(const Key& key);
	void pushRefreshEvent();

	KeyPolicy_ptr policy;
	TMap items;

	mutable std::mutex mapMutex;
};

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
STI::Utils::SynchronizedMap<Key, T>::SynchronizedMap(const KeyPolicy_ptr& Policy) 
{ 
	setPolicy(Policy);
}

template<class Key, class T>
STI::Utils::SynchronizedMap<Key, T>::~SynchronizedMap()
{
	eventHandler.stop();
}

template<class Key, class T>
void STI::Utils::SynchronizedMap<Key, T>::setPolicy(const KeyPolicy_ptr& Policy)
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
void STI::Utils::SynchronizedMap<Key, T>::clearListeners()
{
	eventHandler.clearListeners();
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
	return _remove(key);
}

template<class Key, class T>
bool STI::Utils::SynchronizedMap<Key, T>::_remove(const Key& key)
{
	if (_contains(key)) {
		items.erase(key);
		pushRemoveEvent(key);
	}

	return !_contains(key);
}

template<class Key, class T>
bool STI::Utils::SynchronizedMap<Key, T>::get(const Key& key, T& item) const
{
	std::unique_lock< std::mutex > readLock(mapMutex);
	return _get(key, item);
}

template<class Key, class T>
bool STI::Utils::SynchronizedMap<Key, T>::_get(const Key& key, T& item) const
{
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
	std::unique_lock< std::mutex > readLock(mapMutex);

	_getKeys(keys);
}

template<class Key, class T>
void STI::Utils::SynchronizedMap<Key, T>::_getKeys(std::set<Key>& keys) const
{
	keys.clear();

	for(typename TMap::const_iterator it = items.begin(); it != items.end(); ++it)
	{
		keys.insert(it->first);
	}
}

template<class Key, class T>
void STI::Utils::SynchronizedMap<Key, T>::getValues(std::vector<T>& values) const
{
	std::set<Key> keys;
	getKeys(keys);
	getValues(keys, values);
}

template<class Key, class T>
void STI::Utils::SynchronizedMap<Key, T>::getValues(const std::set<Key>& keys, std::vector<T>& values) const
{
	std::unique_lock< std::mutex > readLock(mapMutex);
	T value;

	for (auto& key : keys) {
		if (_get(key, value)) {
			values.push_back(value);
		}	
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

	std::set<Key> keys;
	_getKeys(keys);

	for(typename std::set<Key>::const_iterator it = keys.begin(); it != keys.end(); ++it) {
		_remove(*it);
	}

	items.clear();
	pushRefreshEvent();
}


#endif

