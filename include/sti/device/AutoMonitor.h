#ifndef STI_DEVICE_AUTOMONITOR_H
#define STI_DEVICE_AUTOMONITOR_H

#include <sti/device/LocalMonitor.h>

#include <functional>
#include <memory>
#include <string>


namespace STI
{
namespace Device
{

class LocalTaskManager;

class AutoMonitor : public LocalMonitor
                  , public std::enable_shared_from_this<AutoMonitor>
{
public:
    using Updater = std::function<STI::Utils::MixedValue(void)>;

    static std::shared_ptr<AutoMonitor> create(
        const std::string& id,
        double updateInterval_s,
        const Updater& updater,
        const std::shared_ptr<LocalTaskManager>& taskManager);

    ~AutoMonitor() override;

    void activate() override;
    void deactivate() override;

private:
	AutoMonitor(
        const std::string& id,
        double updateInterval_s,
        const Updater& updater,
        const std::shared_ptr<LocalTaskManager>& taskManager);

    void initializeTask();
    std::string makeTaskID() const;

    double updateInterval_s;
    Updater updater;
    std::weak_ptr<LocalTaskManager> taskManager;
    std::string taskID;

};


} //Device
} //STI

#endif
