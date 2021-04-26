
#ifndef STI_PYTHON_STIPYLIBDEVICE_H
#define STI_PYTHON_STIPYLIBDEVICE_H


#include "LocalDevice.h"
#include "HubID.h"

#include <string>

namespace STI
{
namespace Python
{


class STIPyLibDevice : public STI::Device::LocalDevice
{
public:

    STIPyLibDevice(const std::string& name, const std::string& address, unsigned short module,
		const STI::Device::DeviceID& serverID, const STI::Network::HubID& serverHubID);

    bool addto(const STI::Network::HubID& target);


    bool getServer(std::shared_ptr<Device>& server);

private:

    STI::Network::HubID serverHubID;
    const STI::Device::DeviceID serverID;

};


} //Python
} //STI

#endif

