
#include <sti/device/LocalAttribute.h>
#include "AttributeRefreshListener.h"
#include <sti/utils/MixedValue.h>
#include <sti/utils/utils.h>

#include <sstream>

using STI::Device::LocalAttribute;
using STI::Device::AttributeRefreshListener;



LocalAttribute::LocalAttribute(const std::string& key, const std::string& initalValue, 
              const std::vector<std::string>& allowedValues)
: LocalAttribute(key, initalValue)
{
    allowedValues_ = allowedValues;
}

LocalAttribute::LocalAttribute(const std::string& key, const std::string& initalValue)
: value_(initalValue)
{
    //parse group
    auto pos = key.find_last_of("::");  //group separator, such as Config::Exposure time
    
    if (pos != std::string::npos) {
        group_ = key.substr(0, pos - 1);
//        key_ = key.substr(pos + 1, key.size());
        key_ = key; //store full key with group
    }
    else {
        key_ = key;
    }

    //Default setter/refresher behavior makes the Attribute act as a simple synchronized key/value 
    //pair storage with no side effects. Setting always succeeds and refreshing returns the last set value.
    setSetter( [](const std::string&) { return true; } );
    // setRefresher( [this](void) -> const std::string& { return this->value_; } );
    setRefresher( [this](void) -> std::string { return this->value_; } );
}

LocalAttribute::~LocalAttribute()
{
}


std::string LocalAttribute::getKey() const
{
    return key_;
}

std::string LocalAttribute::getValue() const
{
    std::unique_lock<std::mutex> attributeLock(attMutex);

    return value_;
}

const std::vector<std::string>& LocalAttribute::getAllowedValues() const
{
    return allowedValues_;
}

std::string LocalAttribute::getGroup() const
{
    return group_;
}

void LocalAttribute::refreshValue()
{
    std::unique_lock<std::mutex> attributeLock(attMutex);
    
    _refresh(value_);
}


bool LocalAttribute::setValue(const std::string& value)
{
    std::unique_lock<std::mutex> attributeLock(attMutex);
    bool success = false;

    if (_isAllowed(value)) {

        std::string oldValue = value_;

        success = setValueCallback(value);
        
        if (success) {
            value_ = value;         //store the successful value so default refresh will work
        } 
        success &= _refresh(oldValue);   //refresh to get actual _value
    }
    return success;
}

bool LocalAttribute::_isAllowed(const std::string& value)
{
    if (allowedValues_.size() ==0) {
        return true;    //no allowed values list specified; all values allowed
    }

    for (auto& v : allowedValues_) {
        if (v.compare(value) == 0) {
            return true;    //value is allowed
        }
    }

    return false;
}

bool LocalAttribute::_refresh(const std::string& oldValue)
{
    bool changed = false;

    value_ = refreshValueCallback();

    //Fires refresh message if value_ has changed.
    if (value_.compare(oldValue) != 0) {
        _fireRefreshEvent();
        changed = true;
    }
    return changed;
}

void LocalAttribute::_fireRefreshEvent()
{
    for (auto& listener : listeners) {
        if (listener != 0) {
            listener->handleAttributeRefreshEvent(key_, value_);
        }
    }
}

LocalAttribute& LocalAttribute::setRefresher(const std::function<std::string(void)>& refresher)
{
    std::unique_lock<std::mutex> attributeLock(attMutex);

    refreshValueCallback = refresher;

    return (*this);
}

LocalAttribute& LocalAttribute::setSetter(const std::function<bool(const std::string&)>& setter)
{
    std::unique_lock<std::mutex> attributeLock(attMutex);

    setValueCallback = setter;

    return (*this);
}

LocalAttribute& LocalAttribute::addMetaData(const std::string& key, const STI::Utils::MixedValue& data)
{
    std::unique_lock<std::mutex> attributeLock(attMutex);

    metaData.addMetaData(key, data);

    return (*this);
}

LocalAttribute& LocalAttribute::addMetaData(const std::string& key, const std::string& data)
{
    std::unique_lock<std::mutex> attributeLock(attMutex);

    STI::Utils::MixedValue value;
    value.setValue(data);

    metaData.addMetaData(key, value);

    return (*this);
}

const STI::Utils::MixedValue& LocalAttribute::getMetaData() const
{
    std::unique_lock<std::mutex> attributeLock(attMutex);
    return metaData.getMetaData();
}

STI::Utils::MixedValue LocalAttribute::getMetaData(const std::string& key) const
{
    std::unique_lock<std::mutex> attributeLock(attMutex);
    return metaData.getMetaData(key);
}

void LocalAttribute::addRefreshListener(AttributeRefreshListener* listener)
{
    std::unique_lock<std::mutex> attributeLock(attMutex);
    listeners.push_back(listener);
}
