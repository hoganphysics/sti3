#ifndef STI_DEVICE_LOCALMONITORMANAGER_H
#define STI_DEVICE_LOCALMONITORMANAGER_H

#include <sti/device/DeviceID.h>
#include <sti/device/MonitorManager.h>
#include <sti/utils/SynchronizedMap.h>

#include <functional>
#include <memory>
#include <string>
#include <vector>


namespace STI
{
namespace Device
{

class DeviceMessageDispatcher;
class LocalMonitorManagerState;


class LocalMonitorManager : public MonitorManager
{
public:
    LocalMonitorManager(const DeviceID& localID, const std::shared_ptr<DeviceMessageDispatcher>& dispatcher);
    ~LocalMonitorManager() override = default;

	void addMonitor(const std::shared_ptr<Monitor>& monitor);
	void removeMonitor(const std::string& id);

	bool getIDs(std::vector<std::string>& ids) override;
	bool getMonitor(const std::string& id, std::shared_ptr<Monitor>& monitor) override;
	bool getMonitors(std::vector<std::shared_ptr<Monitor>>& monitors) override;
	
    MonitorStatus getStatus(const std::string& id) const override;

	STI::Utils::MixedValue getValue(const std::string& id) override;
    bool setValue(const std::string& id, const STI::Utils::MixedValue& value);

	void activate(const std::string& id) override;
	void deactivate(const std::string& id) override;

	void activateAll() override;
	void deactivateAll() override;

	void addListener(const std::shared_ptr<MonitorListener>& listener) override;
    void addValueListener(const std::function<void(const std::string&, const STI::Utils::MixedValue&)>& listener);

private:

	STI::Utils::SynchronizedMap<std::string, std::shared_ptr<Monitor>> monitors;
    std::shared_ptr<LocalMonitorManagerState> state;
};


} //Device
} //STI

#endif
