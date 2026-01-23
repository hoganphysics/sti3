#ifndef STI_NETWORK_REMOTEATTRIBUTEMANAGER_H
#define STI_NETWORK_REMOTEATTRIBUTEMANAGER_H

#include <sti/device/AttributeManager.h>
#include <sti/fwd/MixedValue_fwd.h>
#include "generated/deviceNet.h"
#include <sti/device/DeviceMessageListener.h>
#include <sti/device/DeviceMessage.h>
#include "fwd/DeviceMessageListenerForwarder_fwd.h"
#include "TReferenceHolder.h"
#include <sti/device/DeviceID.h>

#include <memory>
#include <string>
#include <map>
#include <mutex>


namespace STI
{
namespace Network
{

class RemoteAttribute;


class RemoteAttributeManager : public STI::Device::AttributeManager,
                               public STI::TNetwork::TReferenceHolder<STI::TNetwork::TAttributeManager>	//mixin
{
public:

	RemoteAttributeManager(::STI::TNetwork::TAttributeManager_var attributeManager,  
                            const std::shared_ptr<STI::Device::DeviceMessageListenerForwarder>& forwarder,
                            const STI::Device::DeviceID& remoteID);
	~RemoteAttributeManager();
    
    std::string getValue(const std::string& key);
    bool setValue(const std::string& key, const std::string& value);

    bool getAttribute(const std::string& key, std::shared_ptr<STI::Device::Attribute>& attribute);
    void getAttributes(std::vector<std::shared_ptr<STI::Device::Attribute>>& attributes);
    void getAttributes(std::map<std::string, std::string>& attributes);

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
   
    std::string _getUpdatedValue(const std::string& key);
    void setAttributeData(const std::shared_ptr<STI::Network::RemoteAttribute>& attribute);
    std::map<std::string, std::string> attributeData;

    STI::Device::DeviceID remoteID;
    STI::Device::DeviceMessageListenerID listenerID;

    std::shared_ptr<STI::Device::DeviceMessageListenerForwarder> listenerForwarder;

    //::STI::TNetwork::TAttributeManager_var tAttributeManager;		//remote reference

    mutable std::mutex managerMutex;
};


} //Network
} //STI

#endif

