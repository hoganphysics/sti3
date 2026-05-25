#ifndef STI_TNETWORK_TDEVICE_I_H
#define STI_TNETWORK_TDEVICE_I_H

#include "generated/deviceNet.h"

#include <sti/device/Device.h>
#include "TDeviceCollection_i.h"
#include "TDeviceMessageDispatcher_i.h"
#include "TEventEngineScheduler_i.h"
#include "TChannelManager_i.h"
#include "TAttributeManager_i.h"
#include "TPersistenceManager_i.h"
#include "TProfileManager_i.h"
#include "TLogManager_i.h"
#include "TTaskManager_i.h"
#include "TMonitorManager_i.h"
#include "ServantHolder.h"

#include <memory>

namespace STI
{
namespace TNetwork
{


class TDevice_i : public POA_STI::TNetwork::TDevice,
				  public PortableServer::RefCountServantBase
{
public:

	TDevice_i(const std::shared_ptr<STI::Device::Device>& device);
	~TDevice_i();

	::CORBA::Boolean refresh();
	void kill();
	void disable();
	TDeviceCollection_ptr getDeviceCollection();
	TDeviceMessageDispatcher_ptr getMessageDispatcher();
	TEventEngineScheduler_ptr getEngineScheduler();
	TChannelManager_ptr getChannelManager();
	TAttributeManager_ptr getAttributeManager();
	TPersistenceManager_ptr getPersistenceManager();
	TProfileManager_ptr getProfileManager();
	TTaskManager_ptr getTaskManager();
	TLogManager_ptr getLogManager();
	TMonitorManager_ptr getMonitorManager();
	void getMetaData(::STI::TNetwork::TMixedValue_out metaData);
	void getVersions(::STI::TNetwork::TVersionInfoSeq_out versions);
	TDeviceID* getID();

private:

	// TDeviceCollection_i deviceCollectionServant;		//Servant for this Device's collection.
	// TDeviceMessageDispatcher_i messageDispatcherServant;	//Servant for this Device's event dispatcher.
	// TEventEngineScheduler_i eventSchedulerServant;		//Servant for this Device's event scheduler.
	// TChannelManager_i channelManagerServant;
	// TAttributeManager_i attributeManagerServant;
	// TPersistenceManager_i persistenceManagerServant;
	// TProfileManager_i profileManagerServant;
	// TTaskManager_i taskManagerServant;
	// TLogManager_i logManagerServant;

	STI::Network::ServantHolder<TDeviceCollection_i, TDeviceCollection> deviceCollectionServantHolder;
	STI::Network::ServantHolder<TDeviceMessageDispatcher_i, TDeviceMessageDispatcher> messageDispatcherServantHolder;
	STI::Network::ServantHolder<TEventEngineScheduler_i, TEventEngineScheduler> eventSchedulerServantHolder;
	STI::Network::ServantHolder<TChannelManager_i, TChannelManager> channelManagerServantHolder;
	STI::Network::ServantHolder<TAttributeManager_i, TAttributeManager> attributeManagerServantHolder;
	STI::Network::ServantHolder<TPersistenceManager_i, TPersistenceManager> persistenceManagerServantHolder;
	STI::Network::ServantHolder<TProfileManager_i, TProfileManager> profileManagerServantHolder;
	STI::Network::ServantHolder<TTaskManager_i, TTaskManager> taskManagerServantHolder;
	STI::Network::ServantHolder<TLogManager_i, TLogManager> logManagerServantHolder;
	STI::Network::ServantHolder<TMonitorManager_i, TMonitorManager> monitorManagerServantHolder;

	std::shared_ptr<STI::Device::Device> localDevice;	//All calls to servant are forwared to this reference.
};

} //TNetwork
} //STI


#endif
