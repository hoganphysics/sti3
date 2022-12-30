#ifndef STI_DEVICE_LOCALATTRIBUTE_H
#define STI_DEVICE_LOCALATTRIBUTE_H

#include <sti/device/Attribute.h>

#include <sti/utils/MixedValue.h>
#include <sti/utils/MetaData.h>
#include <sti/utils/utils.h>

#include <string>
#include <functional>
#include <mutex>
#include <vector>


namespace STI
{
namespace Device
{

class Attribute;
class AttributeRefreshListener;


class LocalAttribute : public Attribute
{
public:

    LocalAttribute(const std::string& key, const std::string& initalValue);
    LocalAttribute(const std::string& key, const std::string& initalValue, 
              const std::vector<std::string>& allowedValues);
	~LocalAttribute();

	const std::string& getKey() const;
	const std::string& getValue() const;
    const std::vector<std::string>& getAllowedValues() const;
    const std::string& getGroup() const;

    void refreshValue();

    template<typename T>
    bool setValue(const T& value)
    {
        return setValue(STI::Utils::valueToString(value));
    }
    bool setValue(const std::string& value);


    // LocalAttribute& setRefresher(const std::function<const std::string&(void)>& refesher);
    LocalAttribute& setRefresher(const std::function<std::string(void)>& refresher);
    LocalAttribute& setSetter(const std::function<bool(const std::string&)>& setter);
    
    LocalAttribute& addMetaData(const std::string& key, const STI::Utils::MixedValue& data);
    LocalAttribute& addMetaData(const std::string& key, const std::string& data);

    template<typename T>
    LocalAttribute& setRefresher(std::string(T::* refresher)(void), T* self)
    {
        auto fRefresher = [self, refresher](void) { return (self->*refresher)(); };
        return setRefresher(fRefresher);
    }

    template<typename T>
    LocalAttribute& setSetter(bool (T::* setter)(const std::string&), T* self)
    {
        auto fSetter = [self, setter](const std::string& value) { return (self->*setter)(value); };
        return setSetter(fSetter);
    }

	const STI::Utils::MixedValue& getMetaData() const;
	STI::Utils::MixedValue getMetaData(const std::string& key) const;

    void addRefreshListener(AttributeRefreshListener* listener);

private:

    bool _refresh(const std::string& oldValue);     //true if value changed
    void _fireRefreshEvent();
    bool _isAllowed(const std::string& value);

    std::string key_;
    std::string value_;
    std::vector<std::string> allowedValues_;
    std::string group_;     // "::attribute" on global group "::".  "MyGroup::attribute" in group "::MyGroup". "MyGroup::Sub::attribute" in group "::MyGroup::Sub".

    std::function<std::string(void)> refreshValueCallback;
    std::function<bool(const std::string&)> setValueCallback;

    STI::Utils::MetaData metaData;

    std::vector<AttributeRefreshListener*> listeners;

    mutable std::mutex attMutex;
};


} //Device
} //STI

#endif

