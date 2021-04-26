
#include "STIPyServer.h"
#include "ParseTicket.h"
#include "ResultTicket.h"
#include "STIPySeq.h"
#include "STIPyShot.h"
#include "EventEngineScheduler.h"
#include "ShotID.h"

#include <memory>

using STI::Python::STIPyServer;
using STI::Python::STIPyShot;
using STI::Python::STIPySeq;
using STI::Python::ParseTicket;
using STI::Python::ResultTicket;


STIPyServer::STIPyServer()
{
}

STIPyServer::STIPyServer(const std::shared_ptr<STI::Network::NetworkDeviceHub>& libDeviceHub, const std::shared_ptr<STIPyLibDevice>& libDevice)
: libDeviceHub(libDeviceHub), libDevice(libDevice)
{
    std::shared_ptr<STI::Device::Device> server;
    libDevice->getServer(server);
}

void STIPyServer::setChannels(const pybind11::dict& channels)
{
}

std::shared_ptr<STIPyShot> STIPyServer::makeshot()
{
    auto shot = std::make_shared<STIPyShot>(this);
    return shot;
}

std::shared_ptr<STIPyShot> STIPyServer::makeshot(pybind11::object func)
{
    auto shot = std::make_shared<STIPyShot>(this);
    return shot;
}

std::shared_ptr<STIPyShot> STIPyServer::makeshot(pybind11::object func, const pybind11::dict& vars)
{
    auto shot = std::make_shared<STIPyShot>(this);
    return shot;
}

std::shared_ptr<STIPySeq> STIPyServer::makesequence(pybind11::object func)
{
    auto seq = std::make_shared<STIPySeq>();
    return seq;
}


ParseTicket STIPyServer::parse(const std::shared_ptr<STIPyShot>& shot)
{
    std::shared_ptr<STI::Device::Device> server;
    libDevice->getServer(server);

    std::shared_ptr<STI::Engine::EventEngineScheduler> scheduler;
    server->getEngineScheduler(scheduler);

    STI::Engine::ParseID pid;

    scheduler->parse(pid, shot);

    ParseTicket ticket;
    return ticket;
}

ParseTicket STIPyServer::parse(const std::shared_ptr<STIPyShot>& shot, const pybind11::dict& channels)
{
    ParseTicket ticket;
    return ticket;
}

ParseTicket STIPyServer::parse(const std::vector<ParseTicket>& tickets)
{
    ParseTicket ticket;
    return ticket;
}

ResultTicket STIPyServer::play(const ParseTicket& ticket, unsigned repeats)
{
    ResultTicket results;
    return results;
}

ResultTicket STIPyServer::play(const STI::Engine::ParseID& parseID, unsigned repeats)
{
    std::shared_ptr<STI::Device::Device> server;
    libDevice->getServer(server);
    
    std::shared_ptr<STI::Engine::EventEngineScheduler> scheduler;
    server->getEngineScheduler(scheduler);

    STI::Engine::ShotID sid;
    sid.parseID = parseID;

    scheduler->play(sid);

    ResultTicket ticket;
    return ticket;
}

