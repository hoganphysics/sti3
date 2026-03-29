#ifndef STI_DEVICE_MONITORMANAGER_H
#define STI_DEVICE_MONITORMANAGER_H

#include <sti/device/Monitor.h>

#include <vector>
#include <string>
#include <memory>


namespace STI
{
namespace Device
{

class MonitorListener;


class MonitorManager
{
public:

	virtual ~MonitorManager() {}

	virtual bool getIDs(std::vector<std::string>& ids) = 0;
	virtual bool getMonitor(const std::string& id, std::shared_ptr<Monitor>& monitor) = 0;
	virtual bool getMonitors(std::vector<std::shared_ptr<Monitor>>& monitors) = 0;

	virtual MonitorStatus getStatus(const std::string& id) const = 0;

	virtual STI::Utils::MixedValue getValue(const std::string& key) = 0;
    // virtual bool setValue(const std::string& key, const std::string& value) = 0;

	virtual void activate(const std::string& id) = 0;
	virtual void deactivate(const std::string& id) = 0;

	virtual void activateAll() = 0;
	virtual void deactivateAll() = 0;

	virtual void addListener(const std::shared_ptr<MonitorListener>& listener) = 0;
};


} //Device
} //STI

#endif


