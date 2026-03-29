#ifndef STI_NETWORK_REMOTEMONITOR_H
#define STI_NETWORK_REMOTEMONITOR_H

#include <sti/device/Monitor.h>
#include <sti/utils/MetaData.h>

#include <string>


namespace STI
{
namespace Network
{

class RemoteMonitorManager;


class RemoteMonitor : public STI::Device::Monitor
{
public:

    RemoteMonitor(const std::string& id, const std::string& group,
                  STI::Device::MonitorStatus status,
                  const STI::Utils::MixedValue& value,
                  const STI::Utils::MixedValue& metaData);
    ~RemoteMonitor();

    void attachManager(RemoteMonitorManager* manager);

    std::string getID() const override;
    std::string getGroup() const override;
    STI::Device::MonitorStatus getStatus() const override;

    void activate() override;
    void deactivate() override;

    STI::Utils::MixedValue getValue() override;

    const STI::Utils::MixedValue& getMetaData() const override;
    STI::Utils::MixedValue getMetaData(const std::string& key) const override;

    void addListener(const std::shared_ptr<STI::Device::MonitorListener>& listener) override;

private:

    friend class RemoteMonitorManager;

    STI::Device::MonitorStatus getStoredStatus() const;
    const STI::Utils::MixedValue& getStoredValue() const;

    std::string id_;
    std::string group_;
    STI::Device::MonitorStatus status_;
    STI::Utils::MixedValue value_;
    STI::Utils::MetaData metaData_;
    RemoteMonitorManager* remoteManager;
};


} //Network
} //STI

#endif
