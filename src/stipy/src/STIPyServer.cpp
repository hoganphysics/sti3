
#include "STIPyServer.h"
#include "ParseTicket.h"
#include "ResultTicket.h"
#include "STIPySeq.h"
#include "STIPyShot.h"
#include "EventEngineScheduler.h"
#include "ShotID.h"

#include <memory>
#include <iostream>

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
    std::shared_ptr<STI::Engine::EventEngineScheduler> scheduler;
    std::shared_ptr<STI::Engine::Shot> shot;
    
    if (getScheduler(scheduler)) {
        auto evts = std::make_shared<STI::Engine::RawEventVector>();
        shot = scheduler->createShot(evts);
    }
    auto pyShot = std::make_shared<STIPyShot>(shot);
    return pyShot;
}

std::shared_ptr<STIPyShot> STIPyServer::makeshot(pybind11::object func)
{
    return makeshot();
}

std::shared_ptr<STIPyShot> STIPyServer::makeshot(pybind11::object func, const pybind11::dict& vars)
{
    return makeshot();
}

std::shared_ptr<STIPySeq> STIPyServer::makesequence(pybind11::object func)
{
    auto seq = std::make_shared<STIPySeq>();
    return seq;
}

bool STIPyServer::getScheduler(std::shared_ptr<STI::Engine::EventEngineScheduler>& scheduler)
{
    std::shared_ptr<STI::Device::Device> server;
    
    if (libDevice->getServer(server)) {
        return server->getEngineScheduler(scheduler);
    }

    return false;
}

std::shared_ptr<ParseTicket> STIPyServer::parse(const std::shared_ptr<STIPyShot>& pyShot)
{

    STI::Engine::ParseID pid;
    pid.parseTimestamp.timestamp = 1.1;
    
    auto ticket = libDevice->makeParseTicket(pid);
    // return ticket;

    std::shared_ptr<STI::Device::Device> server;
    libDevice->getServer(server);

    if (server == 0) {
        std::cout << "null server" << std::endl;
        return ticket;
    }

    std::shared_ptr<STI::Engine::EventEngineScheduler> scheduler;
    server->getEngineScheduler(scheduler);


    if (scheduler != 0)
        std::cout << "scheduler->parse" << std::endl;
        scheduler->parse(pid, pyShot->getShot());

    // ParseTicket ticket;
    return ticket;
}

std::shared_ptr<ParseTicket> STIPyServer::parse(const std::shared_ptr<STIPyShot>& pyShot, const pybind11::dict& channels)
{
    STI::Engine::ParseID pid;
    pid.parseTimestamp.timestamp = 1.1;
    
    auto ticket = libDevice->makeParseTicket(pid);
    return ticket;
}

std::shared_ptr<ParseTicket> STIPyServer::parse(const std::vector<ParseTicket>& tickets)
{
    STI::Engine::ParseID pid;
    pid.parseTimestamp.timestamp = 1.1;
    
    auto ticket = libDevice->makeParseTicket(pid);
    return ticket;
}

ResultTicket STIPyServer::play(const ParseTicket& ticket)
{
    return play(ticket, 0);
}

ResultTicket STIPyServer::play(const ParseTicket& ticket, unsigned repeats)
{
    return play(ticket.getParseID(), repeats);
}

ResultTicket STIPyServer::play(const STI::Engine::ParseID& parseID, unsigned repeats)
{
    std::shared_ptr<STI::Device::Device> server;
    libDevice->getServer(server);
    
    std::shared_ptr<STI::Engine::EventEngineScheduler> scheduler;
    server->getEngineScheduler(scheduler);

    STI::Engine::ShotID sid;
    sid.parseID = parseID;
    sid.submissionTime.timestamp = 3.1;

    scheduler->play(sid);

    ResultTicket ticket;
    return ticket;
}

