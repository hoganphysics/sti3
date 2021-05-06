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
class ParseTicketManager;
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

private:

    STI::Device::DeviceMessageListenerID schedulerMessageLID;

    std::shared_ptr<ParseTicketManager> ticketManager;

    STI::Network::HubID serverHubID;
    const STI::Device::DeviceID serverID;

};


} //Python
} //STI

#endif

