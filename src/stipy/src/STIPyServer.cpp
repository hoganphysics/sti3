
#include "STIPyServer.h"
#include "PyParseTicket.h"
#include "PyResultTicket.h"
#include "STIPySeq.h"
#include "STIPyShot.h"
#include <sti/engine/EventEngineScheduler.h>
#include <sti/engine/ShotID.h>

#include "StackTraceData.h"
#include <sti/engine/RawEventGroup.h>

#include <sti/utils/LocalFileHolder.h>

#include "STIPyGlobal.h"
#include "LocalShot.h"

#include <chrono>

using STI::Python::STIPyServer;
using STI::Python::STIPyShot;
using STI::Python::STIPySeq;
using STI::Python::PyParseTicket;
using STI::Python::PyResultTicket;
using STI::Engine::StackTraceData;



STIPyServer::STIPyServer(const std::shared_ptr<STI::Network::NetworkDeviceHub>& libDeviceHub, 
                         const std::shared_ptr<STIPyLibDevice>& libDevice, 
                         const STI::Device::DeviceID& serverID)
: libDeviceHub(libDeviceHub), libDevice(libDevice), serverID(serverID)
{
}

STIPyServer::~STIPyServer()
{
//    libDeviceHub->shutdown();
}

void STIPyServer::setChannels(const pybind11::dict& channels)
{
}

std::shared_ptr<STIPyShot> STIPyServer::makeshot()
{
    std::shared_ptr<STI::Engine::EventEngineScheduler> scheduler;
    std::shared_ptr<STI::Device::PersistenceManager> persistenceManager;
    std::shared_ptr<STI::Utils::FileHolderFactory> fileFactory;

    STI::Engine::ShotConfig shotConfig;

    if (getPersistenceManager(persistenceManager)) {
        fileFactory = persistenceManager;
    }
    else {
        fileFactory = std::make_shared<STI::Utils::LocalFileHolderFactory>();
    }

    auto stackTraceData = std::make_shared<StackTraceData>(fileFactory);
    auto eventGroup = std::make_shared<STI::Engine::RawEventGroup>("", "", stackTraceData);
    
    std::shared_ptr<STI::Engine::Shot> shot;

    if (getScheduler(scheduler)) {
        shot = scheduler->createShot(shotConfig, eventGroup);
    }
    auto pyShot = std::make_shared<STIPyShot>(shot);
    return pyShot;
}

std::shared_ptr<STIPyShot> STIPyServer::makeshot(const std::function<void(void)>& func)
{
    auto shot = makeshot();
    auto stipy = STI::Python::STIPyGlobal::getInstance();

    if (stipy != 0) {
        stipy->makeShot(shot, func);        
    }

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
    
    if (libDevice != 0 && libDevice->getServer(server)) {
        return server->getEngineScheduler(scheduler);
    }

    return false;
}

bool STIPyServer::getPersistenceManager(std::shared_ptr<STI::Device::PersistenceManager>& persistenceManager)
{
    std::shared_ptr<STI::Device::Device> server;
    
    if (libDevice != 0 && libDevice->getServer(server)) {
        return server->getPersistenceManager(persistenceManager);
    }

    return false;
}

std::shared_ptr<PyParseTicket> STIPyServer::parse(const std::shared_ptr<STIPyShot>& pyShot)
{
    std::shared_ptr<STI::Engine::EventEngineScheduler> scheduler;
    bool success = false;
    STI::Engine::ParseID pid;

    if (getScheduler(scheduler) && pyShot != 0) {
        pid = scheduler->parse(pyShot->getShot());
        success = true;
    }
    
    auto ticket = libDevice->makeParseTicket(pid);

    if (!success && ticket != 0) {
        ticket->cancel();
    }

    return ticket;
}

std::shared_ptr<PyParseTicket> STIPyServer::parse(const std::shared_ptr<STIPyShot>& pyShot, const pybind11::dict& channels)
{
    STI::Engine::ParseID pid;    
    auto ticket = libDevice->makeParseTicket(pid);
    return ticket;
}

std::shared_ptr<PyParseTicket> STIPyServer::parse(const std::vector<PyParseTicket>& tickets)
{
    STI::Engine::ParseID pid;
    auto ticket = libDevice->makeParseTicket(pid);
    return ticket;
}


std::shared_ptr<PyResultTicket> STIPyServer::play(const std::shared_ptr<PyParseTicket>& ticket)
{
    return play(ticket, 0);
}

std::shared_ptr<PyResultTicket> STIPyServer::play(const std::shared_ptr<PyParseTicket>& ticket, unsigned repeats)
{
    std::shared_ptr<PyResultTicket> resultTicket;

    if (ticket != 0) {

        if (ticket->getStatus() == PyParseTicket::TicketStatus::Complete) {
            resultTicket = play(ticket->getParseID(), repeats);            
        }
        else {
            auto shotID = STI::Engine::ShotID::generateUniqueID(ticket->getParseID());
            resultTicket = libDevice->makeResultTicket(shotID);
            resultTicket->cancel();
        }
    }

    return resultTicket;
}

std::shared_ptr<PyResultTicket> STIPyServer::play(const STI::Engine::ParseID& parseID, unsigned repeats)
{
    STI::Engine::ShotID shotID;
    bool success = false;

    std::shared_ptr<STI::Engine::EventEngineScheduler> scheduler;

    if (getScheduler(scheduler)) {

        STI::Engine::EngineJobSourceID source;
        shotID = scheduler->play(parseID, source);
        success = true;
    }

    auto ticket = libDevice->makeResultTicket(shotID);

    if (!success && ticket != 0) {
        ticket->cancel();
    }

    return ticket;
}


void STIPyServer::cancelAll()
{
    std::shared_ptr<STI::Engine::EventEngineScheduler> scheduler;

    if (getScheduler(scheduler)) {
        scheduler->cancelAll();
    }
}

