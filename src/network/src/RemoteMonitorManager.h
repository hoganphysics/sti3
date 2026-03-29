#ifndef STI_NETWORK_REMOTEMONITORMANAGER_H
#define STI_NETWORK_REMOTEMONITORMANAGER_H

#include "generated/deviceNet.h"
#include <sti/device/MonitorManager.h>
#include <sti/device/DeviceMessageListener.h>
#include <sti/device/DeviceMessage.h>
#include "fwd/DeviceMessageListenerForwarder_fwd.h"
#include "TReferenceHolder.h"
#include <sti/device/DeviceID.h>

#include <memory>
#include <map>
#include <mutex>
#include <string>
#include <vector>


namespace STI
{
namespace Network
{

class RemoteMonitor;

class RemoteMonitorManager : public STI::Device::MonitorManager,
					      public STI::TNetwork::TReferenceHolder<STI::TNetwork::TMonitorManager>	//mixin
{
public:

    RemoteMonitorManager(::STI::TNetwork::TMonitorManager_var manager,
                         const std::shared_ptr<STI::Device::DeviceMessageListenerForwarder>& forwarder,
                         const STI::Device::DeviceID& remoteID);
    ~RemoteMonitorManager();

    bool getIDs(std::vector<std::string>& ids) override;
	bool getMonitor(const std::string& id, std::shared_ptr<STI::Device::Monitor>& monitor) override;
	bool getMonitors(std::vector<std::shared_ptr<STI::Device::Monitor>>& monitors) override;

	STI::Device::MonitorStatus getStatus(const std::string& id) const override;

	STI::Utils::MixedValue getValue(const std::string& id) override;

	void activate(const std::string& id) override;
	void deactivate(const std::string& id) override;

	void activateAll() override;
	void deactivateAll() override;

	void addListener(const std::shared_ptr<STI::Device::MonitorListener>& listener) override;

    bool ping() const;

private:

    friend class RemoteMonitor;

    using ListenerList = std::vector<std::shared_ptr<STI::Device::MonitorListener>>;

    struct MonitorDataTuple
    {
        STI::Device::MonitorStatus status{STI::Device::MonitorStatus::Missing};
        STI::Utils::MixedValue value;
    };

    class MonitorUpdater : public STI::Device::DeviceMessageListener<STI::Device::MonitorUpdateMessage>
    {
    public:

        explicit MonitorUpdater(RemoteMonitorManager* manager) : monitorManager(manager) {}

        void handleMessage(const std::shared_ptr<STI::Device::MonitorUpdateMessage>& mess)
        {
            if (monitorManager != 0) {
                monitorManager->handleMessage(mess);
            }
        }

    private:

        RemoteMonitorManager* monitorManager;
    };

    class MonitorStatusUpdater : public STI::Device::DeviceMessageListener<STI::Device::MonitorStatusUpdateMessage>
    {
    public:

        explicit MonitorStatusUpdater(RemoteMonitorManager* manager) : monitorManager(manager) {}

        void handleMessage(const std::shared_ptr<STI::Device::MonitorStatusUpdateMessage>& mess)
        {
            if (monitorManager != 0) {
                monitorManager->handleMessage(mess);
            }
        }

    private:

        RemoteMonitorManager* monitorManager;
    };

    void setMonitorData(const std::shared_ptr<RemoteMonitor>& monitor);
    STI::Utils::MixedValue getUpdatedValue(const std::string& id) const;
    void handleMessage(const std::shared_ptr<STI::Device::MonitorUpdateMessage>& mess);
    void handleMessage(const std::shared_ptr<STI::Device::MonitorStatusUpdateMessage>& mess);

    mutable std::map<std::string, MonitorDataTuple> monitorData;
    STI::Device::DeviceID remoteID;
    STI::Device::DeviceMessageListenerID valueListenerID;
    STI::Device::DeviceMessageListenerID statusListenerID;
    std::shared_ptr<STI::Device::DeviceMessageListenerForwarder> listenerForwarder;
    std::shared_ptr<const ListenerList> listenerSnapshot;
	mutable std::mutex monitorMutex;
};


} //Network
} //STI


#endif
