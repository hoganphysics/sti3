#ifndef STI_DEVICE_LOCALATTRIBUTEMANAGER_H
#define STI_DEVICE_LOCALATTRIBUTEMANAGER_H

#include <sti/device/AttributeManager.h>
#include <sti/device/DeviceID.h>
#include <sti/utils/SynchronizedMap.h>

#include "AttributeRefreshListener.h"
#include "DeviceMessageGrouper.h"
#include "PersistenceTarget.h"
#include "ProfileTarget.h"

#include <map>
#include <vector>
#include <string>
#include <memory>
#include <atomic>
#include <functional>


namespace STI
{
namespace Device
{

class Attribute;
class LocalAttribute;
class DeviceID;
class DeviceMessageDispatcher;
class AttributeUpdateMessage;


class LocalAttributeManager : public AttributeManager,
                              public AttributeRefreshListener,
                              public PersistenceTarget,
                              public ProfileTarget
{
public:

    LocalAttributeManager(const DeviceID& localID, const std::shared_ptr<DeviceMessageDispatcher>& dispatcher);
    ~LocalAttributeManager();
   
    std::string getValue(const std::string& key);
    bool setValue(const std::string& key, const std::string& value);

    bool getAttribute(const std::string& key, std::shared_ptr<Attribute>& attribute);
    void getAttributes(std::vector<std::shared_ptr<Attribute>>& attributes);

    void getAttributes(std::map<std::string, std::string>& attributes);

    bool addAttribute(const std::shared_ptr<LocalAttribute>& attribute);

    bool loadProfile(const std::shared_ptr<Profile>& profile);
    bool saveProfile(const std::shared_ptr<Profile>& profile);

private:

    //PersistenceTarget
    std::string getFilenameStem();
    std::string getHeader();
    void setPersistenceCallback(const std::function<void(void)>& refresher);
    void setPersistenceData(const std::shared_ptr<STI::Utils::Configuration>& data);
    bool save();
    void load();

    //AttributeRefreshListener
    void handleAttributeRefreshEvent(const std::string& key, const std::string& value);

    const DeviceID localID;		//this device's DeviceID

    STI::Utils::SynchronizedMap<std::string, std::shared_ptr<LocalAttribute>> attributeMap;

    STI::Device::DeviceMessageGrouper<AttributeUpdateMessage> messageGrouper;

    std::function<void(void)> persistenceRefresher;
    std::shared_ptr<STI::Utils::Configuration> persistenceData;
    std::atomic<bool> loading;
};


} //Device
} //STI

#endif

