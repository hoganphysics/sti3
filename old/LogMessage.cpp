
#include "LogMessage.h"
#include "Logger.h"


using STI::Device::LogMessage;



LogMessage::LogMessage(const std::string& message, bool isGroupable)
: isGroupable(isGroupable)
{
    groupedLogs << message;
}
LogMessage::~LogMessage()
{
}

std::string LogMessage::getMessage() const
{
    return groupedLogs.str();
}

bool LogMessage::appendMessage(const LogMessage& mess)
{
    groupedLogs << mess.getMessage();
    return true;
}

bool LogMessage::groupable() const
{
    return isGroupable;
}

LogMessage& LogMessage::get()
{
    return *this;
}
