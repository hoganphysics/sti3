
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
        key_ = key; //store full key with group
    }
    else {
        key_ = key;
    }

    //Default setter/refresher behavior makes the Attribute act as a simple synchronized key/value 
    //pair storage with no side effects. Setting always succeeds and refreshing returns the last set value.
    setSetter( [](const std::string&) { return true; } );
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
    refresh();
}

bool LocalAttribute::refresh()
{
    std::unique_lock<std::mutex> attributeLock(attMutex);
    
    return _refresh(value_);
}

bool LocalAttribute::refreshFrom(const std::string& oldValue)
{
    std::unique_lock<std::mutex> attributeLock(attMutex);

    return _refresh(oldValue);
}


bool LocalAttribute::setValue(const std::string& value)
{
    std::unique_lock<std::mutex> attributeLock(attMutex);
    std::string oldValue = value_;
    bool success = setValueWithoutRefresh(value);

    if (!_isAllowed(value)) {
        return false;
    }

    return _refresh(oldValue) && success;
}

bool LocalAttribute::setValueWithoutRefresh(const std::string& value)
{
    if (!_isAllowed(value)) {
        return false;
    }

    bool success = setValueCallback(value);

    if (success) {
        value_ = value;         //store the successful value so default refresh will work
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

bool LocalAttribute::_refresh(std::string oldValue)
{
    std::string newValue;

    if (refreshValueCallback(newValue)) {
        value_ = newValue;
    }
    else {
        return false;
    }

    //Fires refresh message if value_ has changed.
    if (value_.compare(oldValue) != 0) {
        _fireRefreshEvent();
    }
    return true;    //success, even if value did not change
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
    return setRefresher(
        [refresher](std::string& result) -> bool {
            result = refresher();
            return true;
        });
}

LocalAttribute& LocalAttribute::setRefresher(const std::function<bool(std::string&)>& refresher)
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
