#include <sti/sti.h>

#include "FileTransferDevice.h"

#include <memory>
#include <string>

using STI::Network::NetworkDeviceHub;
using STI::Utils::Configuration;

int main(int argc, char** argv)
{
    Configuration config({
        {"Device Name", "FileTransferDevice"},
        {"IP Address", "localhost"},
        {"Module", "0"},
        {"Target Server", "localhost/0/STI Server"} });

    auto device = std::make_shared<FileTransferDevice>(config);

    std::string nameServiceAddr = "192.168.1.6:2809";
    auto hub = std::make_shared<NetworkDeviceHub>(nameServiceAddr);

    hub->addDevice(device);
    hub->run();

    return 0;
}
