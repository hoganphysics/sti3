#ifndef STI_UTILS_VECTORMAP_H
#define STI_UTILS_VECTORMAP_H

#include <map>
#include <vector>
#include <mutex>


namespace STI
{
namespace Utils
{

// template<typename ID, typename T>
// class VectorMap;


// template<typename ID, typename T>
// class VectorMap : public VectorMap<ID, T>
// {
// public:
//     VectorMap();

// private:
//     std::vector<T> values;
// };


template<typename ID, typename T>
class VectorMap
{
public:

    typedef std::vector<T> vecType;
    typedef std::map<ID, unsigned> mapType;

    VectorMap();
    VectorMap(std::vector<T>& wrappedVec);
    virtual ~VectorMap();

    bool exists(const ID& name) const;
    
    bool getIndex(const ID& name, unsigned& index) const;
    bool get(const ID& name, T& item) const;

    bool at(unsigned index, T& item) const;

    void prepend(const ID& name, const T& item);
    unsigned add(const ID& name, const T& item);
    void rename(const ID& name, const ID& newName);
    void replace(const ID& name, const T& item);

    bool merge(VectorMap<ID, T>& other);

    void clear();

    std::vector<T>& getVec();
    const std::vector<T>& vec() const;

    std::map<ID, unsigned>& getIndexMap();
    const std::map<ID, unsigned>& indexMap() const;

private:

    bool _exists(const ID& name) const;
    bool _at(unsigned index, T& item) const;
    unsigned _add(const ID& name, const T& item);

    std::map<ID, unsigned> indices;
    std::vector<T>& items;

    std::vector<T> items_l;

    mutable std::mutex itemMutex;
};



// template<typename ID, typename T>
// STI::Utils::VectorMap<ID, T>::VectorMap() 
// : VectorMap<ID, T>(values)
// { 
// }

template<typename ID, typename T>
STI::Utils::VectorMap<ID, T>::VectorMap() 
: VectorMap<ID, T>(items_l)
{ 
}


template<typename ID, typename T>
STI::Utils::VectorMap<ID, T>::VectorMap(std::vector<T>& wrappedVec)
: items(wrappedVec)
{
}

template<typename ID, typename T>
STI::Utils::VectorMap<ID, T>::~VectorMap() 
{ 
}


template<typename ID, typename T>
bool STI::Utils::VectorMap<ID, T>::exists(const ID& name) const
{
    std::unique_lock itemLock(itemMutex);

    return _exists(name);
}

template<typename ID, typename T>
bool STI::Utils::VectorMap<ID, T>::_exists(const ID& name) const
{
    auto it = indices.find(name);
    return it != indices.end();
}

template<typename ID, typename T>
bool STI::Utils::VectorMap<ID, T>::getIndex(const ID& name, unsigned& index) const
{
    std::unique_lock itemLock(itemMutex);

    // if (!exists(name)) return false;
    auto it = indices.find(name);
    if (it == indices.end()) return false;
    index = it->second;
    return true;
}

template<typename ID, typename T>
bool STI::Utils::VectorMap<ID, T>::get(const ID& name, T& item) const
{
    std::unique_lock itemLock(itemMutex);

    auto it = indices.find(name);

    if (it != indices.end()) {
        unsigned index = it->second;

        return _at(index, item);
    }
    return false;
}

template<typename ID, typename T>
bool STI::Utils::VectorMap<ID, T>::at(unsigned index, T& item) const
{
    std::unique_lock itemLock(itemMutex);
    return _at(index, item);
}

template<typename ID, typename T>
bool STI::Utils::VectorMap<ID, T>::_at(unsigned index, T& item) const
{
    if (index < items.size()) {
        item = items.at(index);
        return true;
    }
    return false;
}

template<typename ID, typename T>
void STI::Utils::VectorMap<ID, T>::prepend(const ID& name, const T& item)
{
    std::unique_lock itemLock(itemMutex);

    if (_exists(name)) return;   //duplicate

    //shift indices
    for(auto& n : indices) {
        indices[n.first] = n.second + 1;
    }
    indices[name] = 0;
    items.insert(items.begin(), item);
}

template<typename ID, typename T>
void STI::Utils::VectorMap<ID, T>::rename(const ID& name, const ID& newName)
{
    std::unique_lock itemLock(itemMutex);

    if (name == newName) return;

    auto it = indices.find(name);
    if (it == indices.end()) return;    //not found
    if (_exists(newName)) return;    //newName exists

    indices[newName] = it->second;
    indices.erase(it);       
}

template<typename ID, typename T>
void STI::Utils::VectorMap<ID, T>::replace(const ID& name, const T& item)
{
    std::unique_lock itemLock(itemMutex);

    if (_exists(name)) {
        items.at(indices[name]) = item;
    }
    else {
        _add(name, item);
    }
}

template<typename ID, typename T>
unsigned STI::Utils::VectorMap<ID, T>::add(const ID& name, const T& item)
{
    std::unique_lock itemLock(itemMutex);
    return _add(name, item);
}

template<typename ID, typename T>
unsigned STI::Utils::VectorMap<ID, T>::_add(const ID& name, const T& item)
{
    auto it = indices.find(name);
    if (it != indices.end()) {
        //exists
        return it->second;
    }
    items.push_back(item);
    unsigned newIndex = items.size() - 1;
    indices[name] = newIndex;

    return newIndex;
}


template<typename ID, typename T>
bool STI::Utils::VectorMap<ID, T>::merge(VectorMap<ID, T>& other)
{
    std::unique_lock itemLock(itemMutex);

    for (auto& n : other.indices) {
        if (_exists(n.first)) return false;
    }

    unsigned shift = indices.size();
    
    for (auto& n : other.indices) {
        indices[n.first] = n.second + shift;
    }
    
    for (unsigned i = 0; i < other.items.size(); ++i) {
        items.push_back(other.items.at(i));
    }
    return true;
}

template<typename ID, typename T>
std::vector<T>& STI::Utils::VectorMap<ID, T>::getVec()
{
    return items;
}

template<typename ID, typename T>
const std::vector<T>& STI::Utils::VectorMap<ID, T>::vec() const
{
    return items;
}

template<typename ID, typename T>
std::map<ID, unsigned>& STI::Utils::VectorMap<ID, T>::getIndexMap()
{
    return indices;
}

template<typename ID, typename T>
const std::map<ID, unsigned>& STI::Utils::VectorMap<ID, T>::indexMap() const
{
    return indices;
}


template<typename ID, typename T>
void STI::Utils::VectorMap<ID, T>::clear()
{
    std::unique_lock itemLock(itemMutex);

    indices.clear();
    items.clear();
}



} //Utils
} //STI

#endif
