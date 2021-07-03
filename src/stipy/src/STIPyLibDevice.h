#ifndef STI_PYTHON_STIPYLIBDEVICE_H
#define STI_PYTHON_STIPYLIBDEVICE_H

#include "LocalDevice.h"
#include "HubID.h"
#include "ParseID.h"

#include <string>
#include <memory>

namespace STI
{
namespace Python
{

class PyParseTicket;
class PyResultTicket;
class PyParseTicketManager;
class PyResultTicketManager;
class ParseID;


class STIPyLibDevice : public STI::Device::LocalDevice
{
public:
    
    STIPyLibDevice(const std::string& name, const std::string& address, unsigned short module,
		                const STI::Device::DeviceID& serverID, const STI::Network::HubID& serverHubID);
    ~STIPyLibDevice();

    bool addto(const STI::Network::HubID& target);

    bool getServer(std::shared_ptr<Device>& server);

    std::shared_ptr<PyParseTicket> makeParseTicket(const STI::Engine::ParseID& pid);
    std::shared_ptr<PyResultTicket> makeResultTicket(const STI::Engine::ShotID& sid);

private:

    std::shared_ptr<PyParseTicketManager> parseTicketManager;
    std::shared_ptr<PyResultTicketManager> resultTicketManager;

    STI::Device::DeviceMessageListenerID schedulerMessageLID;


    class TicketManagerListener : public STI::Device::DeviceMessageListener<STI::Device::EngineSchedulerMessage>
    {
    public:
        typedef STI::Device::DeviceMessageListener<STI::Device::EngineSchedulerMessage> EngineMessageListener;

        TicketManagerListener(const std::shared_ptr<EngineMessageListener>& parseTicketManager, 
                              const std::shared_ptr<EngineMessageListener>& resultTicketManager)
        : parseTicketManager(parseTicketManager), resultTicketManager(resultTicketManager) {}

        void handleMessage(const std::shared_ptr<STI::Device::EngineSchedulerMessage>& mess);
    
    private:

        std::shared_ptr<EngineMessageListener> parseTicketManager;
        std::shared_ptr<EngineMessageListener> resultTicketManager;
    };

    std::shared_ptr<TicketManagerListener> engineMessageListener;

    STI::Network::HubID serverHubID;
    const STI::Device::DeviceID serverID;

};


} //Python
} //STI

#endif

