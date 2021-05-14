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

class ParseTicket;
class ResultTicket;
class ParseTicketManager;
class ResultTicketManager;
class ParseID;

class STIPyLibDevice : public STI::Device::LocalDevice
{
public:
    
    STIPyLibDevice(const std::string& name, const std::string& address, unsigned short module,
		                const STI::Device::DeviceID& serverID, const STI::Network::HubID& serverHubID);
    ~STIPyLibDevice();

    bool addto(const STI::Network::HubID& target);

    bool getServer(std::shared_ptr<Device>& server);

    std::shared_ptr<ParseTicket> makeParseTicket(const STI::Engine::ParseID& pid);
    std::shared_ptr<ResultTicket> makeResultTicket(const STI::Engine::ShotID& sid);

private:

    STI::Device::DeviceMessageListenerID schedulerMessageLID;
    STI::Device::DeviceMessageListenerID schedulerMessageLID2;

    std::shared_ptr<ParseTicketManager> parseTicketManager;
    std::shared_ptr<ResultTicketManager> resultTicketManager;

    STI::Network::HubID serverHubID;
    const STI::Device::DeviceID serverID;

};


} //Python
} //STI

#endif

