#ifndef STI_NETWORK_REMOTEATTRIBUTEMANAGER_H
#define STI_NETWORK_REMOTEATTRIBUTEMANAGER_H

#include "AttributeManager.h"
#include "fwd/MixedValue_fwd.h"
#include "deviceNet.h"
#include "DeviceMessageListener.h"
#include "fwd/DeviceMessageListenerForwarder_fwd.h"

#include <memory>
#include <string>
#include <map>
#include <mutex>


namespace STI
{
namespace Network
{

class RemoteAttribute;


class RemoteAttributeManager : public STI::Device::AttributeManager
{
public:

	RemoteAttributeManager(::STI::TNetwork::TAttributeManager_ptr attributeManager,  
                            const std::shared_ptr<STI::Device::DeviceMessageListenerForwarder>& forwarder,
                            const STI::Device::DeviceID& remoteID);
	~RemoteAttributeManager();
    
    std::string getValue(const std::string& key);
    bool setValue(const std::string& key, const std::string& value);

    bool getAttribute(const std::string& key, std::shared_ptr<STI::Device::Attribute>& attribute);
    void getAttributes(std::vector<std::shared_ptr<STI::Device::Attribute>>& attributes);

    STI::Utils::MixedValue getMetaData(const std::string& key);
    STI::Utils::MixedValue getMetaData(const std::string& key, const std::string& metaKey);

	const STI::Utils::MixedValue& getMetaData() const;
	STI::Utils::MixedValue getMetaData(const std::string& key) const;

    bool ping() const;

    std::string getUpdatedValue(const std::string& key);

private:
	
    friend class AttributeUpdater;

    //AttributeUpdater
    class AttributeUpdater : public STI::Device::DeviceMessageListener<STI::Device::AttributeUpdateMessage>
    {
    public:

        AttributeUpdater(RemoteAttributeManager* manager) : attributeManager(manager) {}

        void handleMessage(const std::shared_ptr<STI::Device::AttributeUpdateMessage>& mess)
        {
            if (attributeManager == 0) return;
            
            attributeManager->handleMessage(mess);
        }

    private:

        RemoteAttributeManager* attributeManager;
    };

    void handleMessage(const std::shared_ptr<STI::Device::AttributeUpdateMessage>& mess);
   
    void setAttributeData(const std::shared_ptr<STI::Device::Attribute>& attribute);
    std::map<std::string, std::string> attributeData;

    STI::Device::DeviceID remoteID;
    STI::Device::DeviceMessageListenerID listenerID;

    std::shared_ptr<STI::Device::DeviceMessageListenerForwarder> listenerForwarder;

    ::STI::TNetwork::TAttributeManager_var tAttributeManager;		//remote reference

    mutable std::mutex managerMutex;
};


} //Network
} //STI

#endif

