#ifndef STI_DEVICE_LOGMESSAGE_H
#define STI_DEVICE_LOGMESSAGE_H

#include <sti/device/GroupableMessage.h>
#include <sti/utils/EventQueue.h>
#include <sti/device/MessageGrouper.h>
#include <sti/utils/utils.h>

#include <string>
#include <sstream>
#include <functional>
#include <mutex>


namespace STI
{
namespace Device
{

class LocalLogManager;

class Logger;
class LogMessage;


class LogMessage : public STI::Device::GroupableMessage<LogMessage>
{
public:

    LogMessage(const std::string& message, bool isGroupable);
    ~LogMessage();

    // std::string getPrefix() const;
    std::string getMessage() const;
    bool appendMessage(const LogMessage& mess);
	bool groupable() const;
	LogMessage& get();

private:

    bool isGroupable;
    std::stringstream groupedLogs;
};


} //Device
} //STI

#endif
