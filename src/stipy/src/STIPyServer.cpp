
#include "STIPyServer.h"
#include "ParseTicket.h"
#include "ResultTicket.h"
#include "STIPySeq.h"
#include "STIPyShot.h"
#include "EventEngineScheduler.h"
#include "ShotID.h"

#include "STIPyGlobal.h"

#include <chrono>

#include <memory>
#include <iostream>

using STI::Python::STIPyServer;
using STI::Python::STIPyShot;
using STI::Python::STIPySeq;
using STI::Python::ParseTicket;
using STI::Python::ResultTicket;


// STIPyServer::STIPyServer()
// {
// }

STIPyServer::STIPyServer(const std::shared_ptr<STI::Network::NetworkDeviceHub>& libDeviceHub, 
                         const std::shared_ptr<STIPyLibDevice>& libDevice, 
                         const STI::Device::DeviceID& serverID)
: libDeviceHub(libDeviceHub), libDevice(libDevice), serverID(serverID)
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
    auto pyShot = std::make_shared<STIPyShot>(shot, serverID);
    return pyShot;
}

std::shared_ptr<STIPyShot> STIPyServer::makeshot(const std::function<void(void)>& func)
{
    auto shot = makeshot();
    auto stipy = STI::Python::STIPyGlobal::getInstance();

    stipy->makeShot(shot, func);

    return shot;
}

std::shared_ptr<STIPyShot> STIPyServer::makeshot(pybind11::object func, const pybind11::dict& vars)
{
    return makeshot(func);
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
    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    std::chrono::system_clock::duration tp = now.time_since_epoch();
    std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(tp);

    STI::Engine::ParseID pid;
    pid.parseTimestamp.timestamp = ms.count();
    std::cout << "parse time: " << pid.parseTimestamp.timestamp << std::endl;
    
    auto ticket = libDevice->makeParseTicket(pid);

    std::shared_ptr<STI::Engine::EventEngineScheduler> scheduler;

    if (getScheduler(scheduler) && pyShot != 0) {
        scheduler->parse(pid, pyShot->getShot());
    }

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


std::shared_ptr<ResultTicket> STIPyServer::play(const std::shared_ptr<ParseTicket>& ticket)
{
    return play(ticket, 0);
}

std::shared_ptr<ResultTicket> STIPyServer::play(const std::shared_ptr<ParseTicket>& ticket, unsigned repeats)
{
    std::shared_ptr<ResultTicket> rticket;

    if (ticket != 0) {
        rticket = play(ticket->getParseID(), repeats);
    }

    return rticket;
}

std::shared_ptr<ResultTicket> STIPyServer::play(const STI::Engine::ParseID& parseID, unsigned repeats)
{

    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    std::chrono::system_clock::duration tp = now.time_since_epoch();
    std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(tp);

    STI::Engine::ShotID sid;
    sid.parseID = parseID;
    sid.submissionTime.timestamp = ms.count();


    auto ticket = libDevice->makeResultTicket(sid);

    std::shared_ptr<STI::Engine::EventEngineScheduler> scheduler;

    if (getScheduler(scheduler)) {
        scheduler->play(sid);
    }

    // std::shared_ptr<STI::Device::Device> server;
    // libDevice->getServer(server);
    
    // std::shared_ptr<STI::Engine::EventEngineScheduler> scheduler;
    // server->getEngineScheduler(scheduler);


    // ResultTicket ticket;
    return ticket;
}


void STIPyServer::cancelAll()
{
    std::shared_ptr<STI::Engine::EventEngineScheduler> scheduler;

    if (getScheduler(scheduler)) {
        scheduler->cancelAll();
    }
}

