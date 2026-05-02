
#include <sti/sti.h>
#include "ServerDevice.h"
#include "LegacyShotRepository.h"

#include "CLI11/CLI11.hpp"

#include <memory>
#include <iostream>


int main(int argc, char **argv)
{
    //command line parsing
    CLI::App app{"STI Server"};
    argv = app.ensure_utf8(argv);

    std::string configFilename = "default.ini";
    auto configFileOpt = app.add_option("-f,--file", configFilename, "STI Server configuration file")->check(CLI::ExistingFile);

    std::string address = "localhost:2809";
    auto nameserviceOpt = app.add_option("-n,--NameService", address, "NameService address string");

    CLI11_PARSE(app, argc, argv);

    STI::Utils::ConfigFile configFile(configFilename);

    std::shared_ptr<STI::Network::NetworkDeviceHub> hub;

    if (nameserviceOpt->count() > 0) {
        hub = std::make_shared<STI::Network::NetworkDeviceHub>(address);
    }
    else if (configFileOpt->count() > 0) {
        hub = std::make_shared<STI::Network::NetworkDeviceHub>(configFile);
    }
    else {
        hub = std::make_shared<STI::Network::NetworkDeviceHub>("localhost:2809");
    }

    auto server = std::make_shared<STI::Device::ServerDevice>(configFile);

    auto legacyShotRepository = std::make_shared<STI::Engine::LegacyShotRepository>(
        configFile.get<std::string>("Shot Repository", "Path", ".sti/server1")
    );
    server->setShotRepository(legacyShotRepository);

    hub->addDevice(server);

    hub->run(true);

    return 0;
}

