#ifndef STI_DEVICE_LOCALATTRIBUTEMANAGER_H
#define STI_DEVICE_LOCALATTRIBUTEMANAGER_H

#include <sti/device/AttributeManager.h>
#include <sti/device/DeviceID.h>
#include <sti/utils/SynchronizedMap.h>
#include <sti/fwd/ConfigFile_fwd.h>

#include "AttributeRefreshListener.h"
#include "DeviceMessageGrouper.h"
#include "PersistenceTarget.h"
#include "ProfileTarget.h"

#include <map>
#include <set>
#include <vector>
#include <string>
#include <memory>
#include <atomic>
#include <functional>
#include <mutex>


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
    bool refreshValue(const std::string& key) override;
    void refreshValues() override;

    bool getAttribute(const std::string& key, std::shared_ptr<Attribute>& attribute);
    void getAttributes(std::vector<std::shared_ptr<Attribute>>& attributes);

    void getAttributes(std::map<std::string, std::string>& attributes);

    bool addAttribute(const std::shared_ptr<LocalAttribute>& attribute);
    void addAttributeRefreshGroup(const std::vector<std::string>& keys);

    bool loadProfile(const std::shared_ptr<Profile>& profile);
    bool saveProfile(const std::shared_ptr<Profile>& profile);

private:

    //PersistenceTarget
    std::string getFilename();
    std::string getHeader();
    void setPersistenceCallback(const std::function<void(void)>& refresher);
    bool save(const std::string& filename);
    void load(const std::string& filename);

    //AttributeRefreshListener
    void handleAttributeRefreshEvent(const std::string& key, const std::string& value);
    std::vector<std::string> getRefreshTransactionKeys(const std::string& key);
    bool refreshTransaction(const std::string& key);
    bool refreshTransaction(const std::string& key, const std::string& initialOldValue);

    const DeviceID localID;		//this device's DeviceID

    STI::Utils::SynchronizedMap<std::string, std::shared_ptr<LocalAttribute>> attributeMap;
    std::map<std::string, std::set<std::string>> refreshGroupGraph;
    std::mutex refreshGroupMutex;

    STI::Device::DeviceMessageGrouper<AttributeUpdateMessage> messageGrouper;

    std::function<void(void)> persistenceRefresher;
    std::shared_ptr<STI::Utils::ConfigFile> file;
    std::shared_ptr<STI::Utils::Configuration> persistenceData;
    std::atomic<bool> loading;
};


} //Device
} //STI

#endif

