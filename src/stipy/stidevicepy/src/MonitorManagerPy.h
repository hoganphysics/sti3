#ifndef STI_PYTHON_MONITORMANAGERPY_H
#define STI_PYTHON_MONITORMANAGERPY_H

#include <sti/device/MonitorManager.h>
#include <sti/device/Monitor.h>

#include <memory>
#include <string>
#include <vector>


namespace STI
{
namespace Python
{

class MonitorManagerPy : public STI::Device::MonitorManager
{
public:

    explicit MonitorManagerPy(const std::shared_ptr<STI::Device::MonitorManager>& manager);

    std::vector<std::string> getIDs();
    std::shared_ptr<STI::Device::Monitor> getMonitor(const std::string& id);
    std::vector<std::shared_ptr<STI::Device::Monitor>> getMonitors();

    STI::Device::MonitorStatus getStatus(const std::string& id) const override;
    STI::Utils::MixedValue getValue(const std::string& id) override;

    void activate(const std::string& id) override;
    void deactivate(const std::string& id) override;

    void activateAll() override;
    void deactivateAll() override;

private:

    bool getIDs(std::vector<std::string>& ids) override;
    bool getMonitor(const std::string& id, std::shared_ptr<STI::Device::Monitor>& monitor) override;
    bool getMonitors(std::vector<std::shared_ptr<STI::Device::Monitor>>& monitors) override;

    void addListener(const std::shared_ptr<STI::Device::MonitorListener>& listener) override;

    std::shared_ptr<STI::Device::MonitorManager> monitorManager;
};


} //Python
} //STI

#endif
