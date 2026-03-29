#ifndef STI_DEVICE_LOCALMONITOR_H
#define STI_DEVICE_LOCALMONITOR_H

#include <sti/device/Monitor.h>
#include <sti/utils/MetaData.h>

#include <memory>
#include <mutex>
#include <string>
#include <vector>


namespace STI
{
namespace Device
{

// id string uses group name convention: group/id
class LocalMonitor : public Monitor
{
public:
	explicit LocalMonitor(const std::string& id);

	~LocalMonitor() override = default;

	std::string getID() const override;
	std::string getGroup() const override;
	MonitorStatus getStatus() const override;

	void activate() override;
	void deactivate() override;

	void setValue(const STI::Utils::MixedValue& value);
	STI::Utils::MixedValue getValue() override;

    LocalMonitor& addMetaData(const std::string& key, const STI::Utils::MixedValue& data);

    //Metadata can be used for GUI layout, tooltips, units, etc.
	const STI::Utils::MixedValue& getMetaData() const override;
	STI::Utils::MixedValue getMetaData(const std::string& key) const override;

    void addListener(const std::shared_ptr<MonitorListener>& listener) override;

private:
    using ListenerList = std::vector<std::shared_ptr<MonitorListener>>;

    std::string id_;
    std::string group_;
    MonitorStatus status_;
    STI::Utils::MixedValue value_;
    STI::Utils::MetaData metaData;
    std::shared_ptr<const ListenerList> listenerSnapshot;
    mutable std::mutex monitorMutex;
};


} //Device
} //STI

#endif
