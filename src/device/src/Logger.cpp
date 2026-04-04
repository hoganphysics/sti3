#include <sti/device/Logger.h>

#include <sti/LocalDevice.h>
#include <sti/device/Attribute.h>
#include <sti/device/Channel.h>
#include <sti/device/ChannelManager.h>
#include <sti/utils/IntervalTask.h>

#include "LocalAttributeManager.h"
#include "LocalLogManager.h"
#include "LocalPersistenceManager.h"
#include "LocalTaskManager.h"

#include <sstream>
#include <filesystem>
#include <cstdint>
namespace fs = std::filesystem;

using STI::Device::Logger;
using STI::Utils::IntervalTask;
using STI::Device::ChannelManager;
using STI::Device::Channel;
using STI::Utils::MixedValue;


Logger::Logger(const std::string& name, LocalLogManager* manager)
: name(name), manager(manager), messageGrouper(this), logTaskNumber(0)
{
    messageGrouper.setWarmup(1000);      //ms
    messageGrouper.setCooldown(1000);    //ms

    messageGrouper.start();
}

Logger::~Logger()
{
    std::shared_ptr<STI::Device::TaskManager> taskManager;

    if (manager != 0 && manager->localDevice != 0 && manager->localDevice->getTaskManager(taskManager)) {
        for (auto& id : taskIDs) {
            taskManager->removeTask(id);
        }
    }
    
    messageGrouper.stop();
}

std::string Logger::getName() const
{
    return name;
}

int Logger::getNextTaskCount(short channel)
{
    std::unique_lock<std::mutex> logLock(logMutex);

    if (!taskCount.contains(channel)) {
        taskCount[channel] = 0;
    }

    taskCount[channel]++;

    return taskCount[channel];
}

int Logger::getNextTaskCount(const std::string& key)
{
    std::unique_lock<std::mutex> logLock(logMutex);

    if (!attributeCount.contains(key)) {
        attributeCount[key] = 0;
    }

    attributeCount[key]++;

    return attributeCount[key];
}

void Logger::addLogTask(const std::string& timeInterval, const std::function<std::string(void)>& runFunc)
{
    if (manager == 0 || manager->localDevice == 0) return;

    std::stringstream taskID;
    taskID << "Log" << ":" << name << ":" << "Task" << ":" << logTaskNumber;
    logTaskNumber++;

    auto task = std::make_shared<IntervalTask>(taskID.str(), timeInterval, 
        [this, runFunc]() {
            STI::Utils::TimeStamp timeStamp;
            append(makePrefix(timeStamp, {}), runFunc());
        });
    manager->localDevice->addTask(task);
}

void Logger::addReadLogTask(short channel, const std::string& timeInterval, const std::function<MixedValue(void)>& runFunc)
{
    std::shared_ptr<ChannelManager> channelManager;
    std::shared_ptr<Channel> targetChannel;

    manager->localDevice->getChannelManager(channelManager);
    if (channelManager == 0) return;

    if (!channelManager->getChannel(channel, targetChannel)) return;

    // <09/03/2023 13:25:22|read channel|3|'temperature'> 23
    std::vector<std::string> prefixTokens = {"read", STI::Utils::valueToString(targetChannel->getChannelNumber())};

    std::stringstream taskID;
    taskID << "Log" << ":" << name 
            << ":" << "Read channel #" << channel;
    auto chName = targetChannel->getChannelName();
    if (chName != "") {
        taskID << "("  << chName << ")";
        prefixTokens.push_back("'" + chName + "'");
    }
    // Add task count to support multiple tasks on this channel.
    taskID << ":" << "Task #" << getNextTaskCount(channel);

    auto task = std::make_shared<IntervalTask>(taskID.str(), timeInterval, 
        [this, channelManager, targetChannel, runFunc, prefixTokens]() {
            STI::Utils::TimeStamp timeStamp;
            STI::Utils::MixedValue data;
            std::stringstream result;
            
            if (channelManager == 0 || targetChannel == 0) return;

            if (channelManager->readChannel(targetChannel->getChannelNumber(), runFunc(), data)) {
                result << data.print();
            }
            else {
                result << "Failed";
            }

            append(makePrefix(timeStamp, prefixTokens), result.str());
        });
    manager->localDevice->addTask(task);
}

void Logger::addWriteLogTask(short channel, const std::string& timeInterval, const std::function<MixedValue(void)>& runFunc)
{
    std::shared_ptr<ChannelManager> channelManager;
    std::shared_ptr<Channel> targetChannel;

    manager->localDevice->getChannelManager(channelManager);
    if (channelManager == 0) return;

    if (!channelManager->getChannel(channel, targetChannel)) return;

    // <09/03/2023 13:25:22|write channel|3|'setpoint'> target value = 34 | Success | current value = 34
    std::vector<std::string> prefixTokens = {"write", STI::Utils::valueToString(targetChannel->getChannelNumber())};

    std::stringstream taskID;
    taskID << "Log" << ":" << name << ":" << "Write channel #" << channel;
    auto chName = targetChannel->getChannelName();
    if (chName != "") {
        taskID << "("  << chName << ")";
        prefixTokens.push_back("'" + chName + "'");
    }
    // Add task count to support multiple tasks on this channel.
    taskID << ":" << "Task #" << getNextTaskCount(channel);

    auto task = std::make_shared<IntervalTask>(taskID.str(), timeInterval, 
        [this, channelManager, targetChannel, runFunc, prefixTokens]() {
            STI::Utils::TimeStamp timeStamp;
            std::stringstream result;
            MixedValue value = runFunc();
            
            if (channelManager == 0 || targetChannel == 0) return;

            result << "target value = ";
            result << value.print();
            result << " | ";

            if (channelManager->writeChannel(targetChannel->getChannelNumber(), value)) {
                result << "Success";
            }
            else {
                result << "Failed";
            }
            result << " | " << targetChannel->getLastValue().print();

            append(makePrefix(timeStamp, prefixTokens), result.str());
        });
    manager->localDevice->addTask(task);
}


void Logger::addReadLogTask(short channel, const std::string& timeInterval)
{
    addReadLogTask(channel, timeInterval, STI::Utils::MixedValue());
}

void Logger::addReadLogTask(short channel, const std::string& timeInterval, const STI::Utils::MixedValue& value)
{
    addReadLogTask(channel, timeInterval, [value](){ return value; });
}

void Logger::addWriteLogTask(short channel, const std::string& timeInterval, const STI::Utils::MixedValue& value)
{
    addWriteLogTask(channel, timeInterval, [value](){ return value; });
}

std::string Logger::makePrefix(const STI::Utils::TimeStamp& timeStamp, const std::vector<std::string>& annotations)
{
    std::stringstream prefix;

    prefix << "<" << timeStamp.date_YYYY_MM_DD() << " " << timeStamp.time_hh_mm_ss(":");
    for (auto & a : annotations) {
        prefix << "|" << a;
    }
    prefix << ">";

    return prefix.str();
}

void Logger::addAttributeLogTask(const std::string& key, const std::string& timeInterval)
{
    if (manager == 0 || manager->localDevice == 0) return;

    std::shared_ptr<Attribute> attribute;
    manager->localDevice->getAttribute(key, attribute);
    
    if (attribute == 0) return;

    std::stringstream taskID;
    // name:Attribute:key:task
    taskID << "Log" << ":" << name << ":" << "Attribute" << ":" << key;
    // Add task count to support multiple tasks on this attribute.
    taskID << ":" << "Task #" << getNextTaskCount(key);

    auto task = std::make_shared<IntervalTask>(taskID.str(), timeInterval, 
        [this, attribute]() {
            STI::Utils::TimeStamp timeStamp;
            auto prefix = makePrefix(timeStamp, {"attribute", attribute->getKey()});
            append(prefix, attribute->getValue());
        });
    manager->localDevice->addTask(task);
}

std::string Logger::extract()
{
    std::unique_lock<std::mutex> loglock(logMutex);

    auto result = log.str();
    log.clear();
    log.str("");
    return result;
}

void Logger::clear()
{
    std::unique_lock<std::mutex> loglock(logMutex);
    log.clear();
    log.str("");
}

std::string Logger::getLog() const
{
    std::unique_lock<std::mutex> loglock(logMutex);
    return log.str();
}

Logger& Logger::operator<<(manip1 fp)
{
    std::unique_lock<std::mutex> loglock(logMutex);
    auto mess = std::make_shared<LogStreamMessage>(fp);
    messageGrouper.addMessage(mess);
    return *this;
}

Logger& Logger::operator<<(manip2 fp)
{
    std::unique_lock<std::mutex> loglock(logMutex);
    auto mess = std::make_shared<LogStreamMessage>(fp);
    messageGrouper.addMessage(mess);
    return *this;
}

Logger& Logger::operator<<(manip3 fp)
{
    std::unique_lock<std::mutex> loglock(logMutex);
    auto mess = std::make_shared<LogStreamMessage>(fp);
    messageGrouper.addMessage(mess);
    return *this;
}

void Logger::LogMessageGrouper::dispatchMessage(const std::shared_ptr<LogStreamMessage>& mess)
{
    if (mess == 0) return;
    if (logger == 0) return;

    logger->append(mess->getMessage());
}

void Logger::append(const std::string& input)
{
    STI::Utils::TimeStamp timeStamp;
    
    append(makePrefix(timeStamp, {}), input);
}


void Logger::append(const std::string& prefix, const std::string& input)
{
    std::unique_lock<std::mutex> loglock(logMutex);
    log << prefix << " ";
    log << input;
    log << std::endl;

    sendLogWriteMessage();
}

void Logger::sendLogWriteMessage()
{
    auto mess = std::make_shared<LocalLogManager::LogWriteMessage>(name);
    manager->logWriterMessageGrouper.addMessage(mess);
}

std::string Logger::getNextLogFilename(const std::string& targetDirectory)
{
    fs::path nextLogFilename;
    fs::path newLogFilenamePath = targetDirectory;

    bool fileCheck = false;
    unsigned i = 0;
    const auto maxFileSize = (manager != nullptr)
        ? manager->getMaxLogFileSizeBytes()
        : LocalLogManager::DefaultMaxLogFileSizeBytes;
    const auto pendingWriteSize = static_cast<std::uintmax_t>(log.str().size());

    do {
        auto filename = manager->makeLogFilename(name, i);

        nextLogFilename = newLogFilenamePath / filename;

        if (fs::exists(nextLogFilename)) {
            // Keep each log file under the configured size after the pending buffer is appended.
            const auto currentFileSize = fs::file_size(nextLogFilename);
            fileCheck = currentFileSize + pendingWriteSize <= maxFileSize;
        }
        else {
            fileCheck = true;
        }
        i++;
    }
    while (!fileCheck);

    return nextLogFilename.string();
}

bool Logger::save(const std::string& targetDirectory)
{
    std::unique_lock<std::mutex> loglock(logMutex);

    auto newLogFilename = getNextLogFilename(targetDirectory);

    if (logFile == 0 || !logFile->is_open() || newLogFilename != logFilename) {
        //new log file
        logFile = std::make_unique<std::ofstream>(newLogFilename, std::ios::app);   //append
        logFilename = newLogFilename;
    }
    
    if (logFile == 0) return false;
    if (!logFile->is_open()) return false;

    (*logFile) << log.str();
    logFile->close();
    log.str("");
    log.clear();

    return true;

    if (manager == 0) return false;
    if (manager->localPersistenceManager == 0) return false;
    
    std::string newLogBasePath;
    std::string newLogDevicePath;
    STI::Utils::TimeStamp timestamp;

    if (!manager->localPersistenceManager->makeLogPath(timestamp, newLogBasePath)) return false;

    //modify log record
    
    if (!manager->localPersistenceManager->makeLogPath(timestamp, manager->localDevice->getID(), newLogDevicePath)) return false;

    return false;
}
