#ifndef STI_DEVICE_MONITOR_H
#define STI_DEVICE_MONITOR_H

#include <sti/utils/MixedValue.h>

#include <memory>
#include <string>
#include <vector>

namespace STI
{
namespace Device
{

enum class MonitorStatus { Active, Inactive, Missing };

class MonitorListener;


class Monitor
{
public:

	virtual ~Monitor() {}

	virtual std::string getID() const = 0;
	virtual std::string getGroup() const = 0;
	virtual MonitorStatus getStatus() const = 0;

	virtual void activate() = 0;
	virtual void deactivate() = 0;

	// virtual void setValue(const STI::Utils::MixedValue& value) = 0;
	virtual STI::Utils::MixedValue getValue() = 0;

    //Metadata can be used for GUI layout, tooltips, units, etc.
	virtual const STI::Utils::MixedValue& getMetaData() const = 0;
	virtual STI::Utils::MixedValue getMetaData(const std::string& key) const = 0;

    virtual void addListener(const std::shared_ptr<MonitorListener>& listener) = 0;
};


} //Device
} //STI

#endif
