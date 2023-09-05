#ifndef STI_DEVICE_LOGGER_H
#define STI_DEVICE_LOGGER_H

#include <sti/device/GroupableMessage.h>
#include <sti/device/MessageGrouper.h>

#include <sti/utils/MixedValue.h>
#include <sti/utils/TimeStamp.h>
#include <sti/utils/utils.h>


#include <string>
#include <sstream>
#include <functional>
#include <mutex>
#include <memory>
#include <fstream>


namespace STI
{
namespace Device
{

class LocalLogManager;

// Examples:
// <10:24:34 | read | ch 2> MixedValue()
// <10:24:34 | attribute | key> value

class Logger
{
public:

    Logger(const std::string& name, LocalLogManager* manager);
    ~Logger();

    std::string getName() const;

    void addLogTask(const std::string& timeInterval, const std::function<std::string(void)>& runFunc);
    void addReadLogTask(short channel, const std::string& timeInterval);

    // template<typename T>
    // void addReadLogTask(short channel, const std::string& timeInterval, const T& value)
    // {
    //     STI::Utils::MixedValue mixedValue;
    //     mixedValue.setValue(value);
    //     addReadLogTask(channel, timeInterval, mixedValue);
    // }
    void addReadLogTask(short channel, const std::string& timeInterval, const STI::Utils::MixedValue& value);
    void addReadLogTask(short channel, const std::string& timeInterval, const std::function<STI::Utils::MixedValue(void)>& runFunc);

    // template<typename T>
    // void addWriteLogTask(short channel, const std::string& timeInterval, const T& value)
    // {
    //     STI::Utils::MixedValue mixedValue;
    //     mixedValue.setValue(value);
    //     addWriteLogTask(channel, timeInterval, mixedValue);
    // }
    void addWriteLogTask(short channel, const std::string& timeInterval, const STI::Utils::MixedValue& value);
    void addWriteLogTask(short channel, const std::string& timeInterval, const std::function<STI::Utils::MixedValue(void)>& runFunc);

    void addAttributeLogTask(const std::string& key, const std::string& timeInterval);
    //addReadLogTask(0, "00:05:00");
    //addAttributeLogTask("key", "00:05:00");
    //addReadLogTask(0, "00:05:00", [this](){ read(0, 1.4, result); log() << result << std::endl; })
    //addWriteLogTask(1, "00:05:00", [this](){ write(0, 5.4); log() << channel.getLastValue() << std::endl; })
    //addAttributeLogTask("key", "00:05:00", [this](){ log() << getAttribute(key) << std::endl; })

    template<typename T>
    Logger& operator<<(const T& input)
    {
        std::unique_lock<std::mutex> loglock(logMutex);
        auto mess = std::make_shared<LogStreamMessage>(STI::Utils::valueToString(input));
        messageGrouper.addMessage(mess);

        return (*this);
    }


    // overloads for manipulators
    typedef std::ostream& (*manip1)(std::ostream&);
    typedef std::basic_ios<std::ostream::char_type, std::ostream::traits_type> ios_type;
    typedef ios_type& (*manip2)(ios_type&);
    typedef std::ios_base& (*manip3)(std::ios_base&);

    Logger& operator<<(manip1 fp);
    Logger& operator<<(manip2 fp);
    Logger& operator<<(manip3 fp);

private:

    // For logs entered using stream operations (<<), group recent calls together into a single log message
    class LogStreamMessage : public STI::Device::GroupableMessage<LogStreamMessage>
    {
    public:

        LogStreamMessage(const std::string& message)
        {
            groupedLogs << message;
        }
        LogStreamMessage(manip1 fp)
        {
            groupedLogs << fp;
        }
        LogStreamMessage(manip2 fp)
        {
            groupedLogs << fp;
        }
        LogStreamMessage(manip3 fp)
        {
            groupedLogs << fp;
        }
        ~LogStreamMessage() {}

        std::string getMessage() const { return groupedLogs.str(); }
        bool appendMessage(const LogStreamMessage& mess)
        {
            groupedLogs << mess.getMessage();
            return true;
        }
        bool groupable() const { return true; }
        LogStreamMessage& get() { return *this; }

    private:
        std::stringstream groupedLogs;
    };

    class LogMessageGrouper : public MessageGrouper<LogStreamMessage>
    {
    public:

        LogMessageGrouper(Logger* logger) : logger(logger) {}
        void dispatchMessage(const std::shared_ptr<LogStreamMessage>& mess);

    private:

        Logger* logger;
    };



    template<typename T>
    void append(const std::string& prefix, const T& input)
    {
        append(prefix, STI::Utils::valueToString(input));
    }
    void append(const std::string& prefix, const std::string& input);
    void append(const std::string& input);


    std::string extract();
    void clear();

    std::string getLog() const;

    void sendLogWriteMessage();

    std::string makePrefix(const STI::Utils::TimeStamp& timeStamp, const std::vector<std::string>& annotations);

    std::string getNextLogFilename(const std::string& targetDirectory);

    // //PersistenceTarget
    // std::string getFilename();
    // // void setLoadFilename(const std::string& filename);
    // void setPersistenceCallback(const std::function<void(void)>& refresher);
    // bool save(const std::string& filename);
    // void load(const std::string& filename);

    friend class LocalLogManager;
    bool save(const std::string& targetDirectory);

    // std::string makeLogPrefix();

    std::string name;
    std::stringstream log;
    unsigned logTaskNumber;

    // std::ofstream logFile;
    std::unique_ptr<std::ofstream> logFile;
    std::string logFilename;    //today's log file

    std::vector<std::string> taskIDs;
    
    LogMessageGrouper messageGrouper;

    LocalLogManager* manager;

    mutable std::mutex logMutex;
};


} //Device
} //STI

#endif
