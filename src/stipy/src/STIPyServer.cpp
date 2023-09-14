
#include "STIPyServer.h"

#include <sti/engine/AddSequenceStatus.h>
#include <sti/engine/EventEngineScheduler.h>
#include <sti/engine/ParsedVar.h>
#include <sti/engine/ParseJobStatus.h>
#include <sti/engine/PlayJobStatus.h>
#include <sti/engine/RawEventGroup.h>
#include <sti/engine/Sequence.h>
#include <sti/engine/ShotID.h>

#include <sti/utils/LocalFileHolder.h>

#include "LocalShot.h"
#include "PyParseTicket.h"
#include "PyResultTicket.h"
#include <sti/engine/StackTraceData.h>
#include "STIPyGlobal.h"
#include "STIPyShot.h"

#include <chrono>

using STI::Python::STIPyServer;
using STI::Python::STIPyShot;
using STI::Python::STIPySeq;
using STI::Python::PyParseTicket;
using STI::Python::PyResultTicket;
using STI::Engine::StackTraceData;
using STI::Engine::ParseJobStatus;
using STI::Engine::PlayJobStatus;
using STI::Engine::Sequence;
using STI::Engine::SequenceEntryID;
using STI::Engine::ParsedVar;


STIPyServer::STIPyServer(const std::shared_ptr<STI::Network::NetworkDeviceHub>& libDeviceHub, 
                         const std::shared_ptr<STIPyLibDevice>& libDevice, 
                         const STI::Device::DeviceID& serverID)
: libDeviceHub(libDeviceHub), libDevice(libDevice), serverID(serverID)
{
    std::shared_ptr<STI::Device::Device> server;

    if (libDevice != 0 && libDevice->getServer(server)) {
        this->setDevice(server);
    }
}

STIPyServer::~STIPyServer()
{
    //libDeviceHub->shutdown();
}

void STIPyServer::setChannels(const pybind11::dict& channels)
{
}

std::shared_ptr<STIPyShot> STIPyServer::makeshot()
{
    std::shared_ptr<STI::Engine::EventEngineScheduler> scheduler;
    std::shared_ptr<STI::Device::PersistenceManager> persistenceManager;
    std::shared_ptr<STI::Utils::FileServer> fileServer;

    STI::Engine::ShotConfig shotConfig;
    if (libDevice != 0) {
        shotConfig.jobSourceID.machine = libDevice->getID().getAddress();
        shotConfig.jobSourceID.user = getUserName();
    }
    

    // if (getPersistenceManager(persistenceManager)) {
    //     fileFactory = persistenceManager;
    // }
    // else {
    //     fileFactory = std::make_shared<STI::Utils::LocalFileHolderFactory>();
    // }


    // if (getPersistenceManager(persistenceManager)) {
    //     persistenceManager->getFileServer(fileServer);
    // }
    // else {
    //     // fileFactory = std::make_shared<STI::Utils::LocalFileHolderFactory>();
    // }

    bool success = libDevice->getFileServer(fileServer);
    // if (libDevice->getPersistenceManager(persistenceManager)) {
    //     bool success = persistenceManager->getFileServer(fileServer);
    // }

    auto stackTraceData = std::make_shared<StackTraceData>(libDevice->getID(), fileServer);
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

std::shared_ptr<STIPyShot> STIPyServer::makeshot(const std::function<void(void)>& func, const std::set<ParsedVar>& vars)
{
    auto shot = makeshot();
    auto stipy = STI::Python::STIPyGlobal::getInstance();

    shot->group()->bindVars(vars);

    if (stipy != 0) {
        stipy->makeShot(shot, func);        
    }

    return shot;
}



// std::shared_ptr<Sequence> makesequence()
// {
//     auto seq = std::make_shared<Sequence>();
//     return seq;
// }

// std::shared_ptr<Sequence> STIPyServer::makesequence(pybind11::object func)
// {
//     auto seq = std::make_shared<STIPySeq>();
//     return seq;
// }

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
    STI::Engine::ParseJobStatus parseJobStatus;

    if (getScheduler(scheduler) && pyShot != 0) {
        parseJobStatus = scheduler->parse(pyShot->getShot());
        success = true;
    }
    
    if (libDevice == 0) return 0;

    auto ticket = libDevice->makeParseTicket(parseJobStatus.pid);

    if (ticket == 0) return ticket;

    if (!success) {
        ticket->cancel();
    }
    else if (parseJobStatus.status == STI::Engine::EngineJobStatus::Deferred) {
        ticket->defer();
    }

    return ticket;
}

std::shared_ptr<PyParseTicket> STIPyServer::parse(const std::shared_ptr<STIPyShot>& pyShot, const SequenceEntryID& sequenceEntryID)
{
    std::shared_ptr<STI::Engine::EventEngineScheduler> scheduler;
    bool success = false;
    STI::Engine::ParseJobStatus parseJobStatus;

    if (getScheduler(scheduler) && pyShot != 0) {
        parseJobStatus = scheduler->parse(pyShot->getShot(), sequenceEntryID);
        success = true;
    }
    
    if (libDevice == 0) return 0;

    auto ticket = libDevice->makeParseTicket(parseJobStatus.pid);

    if (ticket == 0) return ticket;

    if (!success) {
        ticket->cancel();
    }
    else if (parseJobStatus.status == STI::Engine::EngineJobStatus::Deferred) {
        ticket->defer();
    }

    return ticket;
}


std::shared_ptr<PyParseTicket> STIPyServer::parse(const std::shared_ptr<STIPyShot>& pyShot, const pybind11::dict& channels)
{
    if (libDevice == 0) return 0;

    STI::Engine::ParseID pid;    
    auto ticket = libDevice->makeParseTicket(pid);
    return ticket;
}

std::shared_ptr<PyParseTicket> STIPyServer::parse(const std::vector<PyParseTicket>& tickets)
{
    if (libDevice == 0) return 0;

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
            if (libDevice == 0) return 0;

            STI::Engine::EngineJobSourceID source;
            
            source.machine = libDevice->getID().getAddress();
            source.user = username = getUserName();

            auto shotID = STI::Engine::ShotID::generateUniqueID(ticket->getParseID(), source);
            resultTicket = libDevice->makeResultTicket(shotID);
            resultTicket->cancel();
        }
    }

    return resultTicket;
}

std::shared_ptr<PyResultTicket> STIPyServer::play(const STI::Engine::ParseID& parseID, unsigned repeats)
{
    STI::Engine::PlayJobStatus playJobStatus;
    bool success = false;

    std::shared_ptr<STI::Engine::EventEngineScheduler> scheduler;

    if (getScheduler(scheduler)) {

        STI::Engine::EngineJobSourceID source;
        if (libDevice != 0) {
            source.machine = libDevice->getID().getAddress();
            source.user = username = getUserName();
        }
        
        playJobStatus = scheduler->play(parseID, source);
        success = true;
    }

    if (libDevice == 0) return 0;

    auto ticket = libDevice->makeResultTicket(playJobStatus.sid);

    if (ticket == 0) return ticket;

    if (!success) {
        ticket->cancel();
    }
    else if (playJobStatus.status == STI::Engine::EngineJobStatus::Deferred) {
        ticket->defer();
    }

    return ticket;
}

STI::Engine::SequenceID STIPyServer::parse(const std::shared_ptr<STI::Engine::Sequence>& seq)
{
    std::shared_ptr<STI::Engine::EventEngineScheduler> scheduler;
    bool success = false;
    STI::Engine::AddSequenceStatus addSequenceStatus;

    if (getScheduler(scheduler) && seq != 0) {
        STI::Engine::EngineJobSourceID source;
        addSequenceStatus = scheduler->addSequence(seq, source);
        success = true;
    }
    
    if (addSequenceStatus.status == STI::Engine::EngineJobStatus::Deferred) {
    }

    return addSequenceStatus.seqid;
}


void STIPyServer::cancelAll()
{
    std::shared_ptr<STI::Engine::EventEngineScheduler> scheduler;

    if (getScheduler(scheduler)) {
        scheduler->cancelAll();
    }
}

void STIPyServer::setUserName(const std::string& name)
{
    username = name;
}

std::string STIPyServer::getUserName() const
{
    return username;
}

std::string STIPyServer::printNetwork()
{
    std::string result;
    if (libDeviceHub != 0) {
        result = libDeviceHub->printNetwork();
    }
    return result;
}

std::string STIPyServer::printNetwork(const std::string& baseContext)
{
    std::string result;
    if (libDeviceHub != 0) {
        result = libDeviceHub->printNetwork(baseContext);
    }
    return result;
}
