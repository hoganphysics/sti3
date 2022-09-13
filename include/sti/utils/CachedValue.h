#ifndef STI_UTILS_CACHEDVALUE_H
#define STI_UTILS_CACHEDVALUE_H

#include <mutex>


namespace STI
{
namespace Utils
{


template<typename T>
class CachedValue
{
public:

    CachedValue() : cached(false) {}
    ~CachedValue() {}

    bool operator==(const T& other) const
    {
        std::unique_lock<std::mutex> cLock(cacheMutex);
        if (cached) {
            return (other == cachedValue);
        }
        return false;
    }
    
    bool isCached() const
    {
        std::unique_lock<std::mutex> cLock(cacheMutex);
        return cached;
    }
    
    bool getValue(T& value) const
    {
        std::unique_lock<std::mutex> cLock(cacheMutex);
        if (cached) {
            value = cachedValue;
        }
        return cached;
    }

    T get() const
    {
        std::unique_lock<std::mutex> cLock(cacheMutex);
        return cachedValue;
    }

    void set(const T& value)
    {
        std::unique_lock<std::mutex> cLock(cacheMutex);
        cached = true;
        cachedValue = value;
    }

    void reset()
    {
        cached = false;
    }

private:

    bool cached;
    T cachedValue;

    mutable std::mutex cacheMutex;
};


} //Utils
} //STI

#endif

