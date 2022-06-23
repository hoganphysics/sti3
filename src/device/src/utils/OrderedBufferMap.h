#ifndef STI_UTILS_ORDEREDBUFFERMAP_H
#define STI_UTILS_ORDEREDBUFFERMAP_H

#include <sti/utils/SynchronizedMap.h>

#include <deque>
#include <memory>
#include <set>
#include <algorithm>

/*
This template class is an insertion-ordered buffer of fixed length, with arbitrary buffer access.
The class stores a set of items in a buffer by (Key, Value) pairs. Once the buffer 
is full, the oldest item is removed whenever a new item is added. Arbitary access to the buffer 
elements (and thread-safety) is implemented using a SynchronizedMap.
*/

namespace STI
{
namespace Utils
{

template<class Key>
class OrderedBufferMapPolicy : public SynchronizedMapPolicy<Key>
{
public:
	
	OrderedBufferMapPolicy(std::deque<Key>* buffer_keys) : buffer_keys(buffer_keys) {}

	bool include(const Key& key) const 
	{ 
		typename std::deque<Key>::iterator it = std::find(buffer_keys->begin(), buffer_keys->end(), key);
		return (it != buffer_keys->end());		// true if key is in buffer_keys
	}
	bool replace(const Key& oldKey, const Key& newKey) const { return (oldKey == newKey); }

private:
	std::deque<Key>* buffer_keys;	//ordered buffer of keys (newest at front)

};


template<class Key, class T>
class OrderedBufferMap
{
public:

	OrderedBufferMap(unsigned size);
	virtual ~OrderedBufferMap() {}

	void setMaxSize(unsigned size);
	unsigned size() const;

	bool contains(const Key& key) const;

	bool get(const Key& key, T& item) const;
	void getKeys(std::set<Key>& keys) const;
	bool add(const Key& key, T item);
	bool remove(const Key& key);
	bool addAndRemove(const Key& key, T newItem, T& oldItem);

	void clear();


private:
	
	void trimToSize();	

	unsigned max_size;

	SynchronizedMap<Key, T> buffer;	//the actual data being buffered
	//std::shared_ptr<OrderedBufferMapPolicy<Key>> bufferPolicy;

	std::deque<Key> buffer_keys;	//ordered buffer of keys
	mutable std::mutex dequeMutex;

};


} // UTILS
} // STI

template<class Key, class T>
STI::Utils::OrderedBufferMap<Key, T>::OrderedBufferMap(unsigned size)
{
	//The policy enforces that only Keys in the ordered buffer deque are kept.
	//When the buffer reaches max size, the oldest entries are dropped first when
	//new entries arrive (or the buffer is resized).
	auto bufferPolicy = std::make_shared<OrderedBufferMapPolicy<Key>>(&buffer_keys);
	buffer.setPolicy(bufferPolicy);

	setMaxSize(size);
}


template<class Key, class T>
void STI::Utils::OrderedBufferMap<Key, T>::setMaxSize(unsigned size)
{
	std::unique_lock<std::mutex> writeLock(dequeMutex);

	max_size = size;
	trimToSize();
}

template<class Key, class T>
unsigned STI::Utils::OrderedBufferMap<Key, T>::size() const
{
	return buffer.size();
}


template<class Key, class T>
bool STI::Utils::OrderedBufferMap<Key, T>::contains(const Key& key) const
{
	return buffer.contains(key);
}

template<class Key, class T>
bool STI::Utils::OrderedBufferMap<Key, T>::get(const Key& key, T& item) const
{
	return buffer.get(key, item);
}

template<class Key, class T>
void STI::Utils::OrderedBufferMap<Key, T>::getKeys(std::set<Key>& keys) const
{
	buffer.getKeys(keys);
}

template<class Key, class T>
bool STI::Utils::OrderedBufferMap<Key, T>::add(const Key& key, T item)
{
	std::unique_lock<std::mutex> writeLock(dequeMutex);

	bool success;
	buffer_keys.push_front(key);		//add new key to front
	
	success = buffer.add(key, item);	//attempt to add item to buffer

	if (success) {
		trimToSize();					//remove oldest key(s) from back
	}
	else {
		buffer_keys.pop_front();	//add failed; remove the new key from front
	}
	return success;
}

template<class Key, class T>
bool STI::Utils::OrderedBufferMap<Key, T>::addAndRemove(const Key& key, T newItem, T& oldItem)
{
	std::unique_lock<std::mutex> writeLock(dequeMutex);

	bool oldItemValid = false;

	buffer_keys.push_front(key);		//add new key to front
	buffer.add(key, newItem);	//attempt to add item to buffer

	if (buffer_keys.size() > max_size) {

		auto lastKey = buffer_keys.back();
		oldItemValid = buffer.get(lastKey, oldItem);

	}

	trimToSize();

	return oldItemValid;

}

template<class Key, class T>
bool STI::Utils::OrderedBufferMap<Key, T>::remove(const Key& key)
{
	std::unique_lock<std::mutex> writeLock(dequeMutex);

	typename std::deque<Key>::iterator it = std::find(buffer_keys.begin(), buffer_keys.end(), key);

	if ((it != buffer_keys.end()) && buffer.remove(key)) {
		buffer_keys.erase(it);
		return true;
	}

	return false;
}


template<class Key, class T>
void STI::Utils::OrderedBufferMap<Key, T>::clear()
{
	std::unique_lock<std::mutex> writeLock(dequeMutex);
	buffer_keys.clear();
	
	buffer.clear();
}

template<class Key, class T>
void STI::Utils::OrderedBufferMap<Key, T>::trimToSize()
{
	while (buffer_keys.size() > max_size) {
		buffer_keys.pop_back();
	}

	buffer.cleanup();	//removes items that are not in buffer_keys
}


#endif
