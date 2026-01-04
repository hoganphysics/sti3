
#include "STIPyServer.h"

#include <sti/engine/AddSequenceStatus.h>
#include <sti/engine/EventEngineScheduler.h>
#include <sti/engine/ParsedVar.h>
#include <sti/engine/ParseJobStatus.h>
#include <sti/engine/PlayJobStatus.h>
#include <sti/engine/RawEventGroup.h>
#include <sti/engine/Sequence.h>
#include <sti/engine/ShotID.h>
#include <sti/engine/StackTraceData.h>

#include <sti/utils/LocalFileHolder.h>

#include "LocalShot.h"
#include "PyParseTicket.h"
#include "PyResultTicket.h"
#include "STIPyGlobal.h"
#include "STIPyShot.h"

#include <chrono>

#include <pybind11/pybind11.h>

namespace py = pybind11;

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
    // libDevice->disable();
    
    disconnect();
}

void STIPyServer::disconnect()
{
    if (libDevice != nullptr) {
        libDevice->kill();
    }

    if (libDeviceHub != nullptr) {
        libDeviceHub->shutdown();
    }

    libDevice = 0;
    libDeviceHub = 0;
}

void STIPyServer::setChannels(const pybind11::dict& channels)
{
}

// std::shared_ptr<STIPyShot> STIPyServer::makeshot()
// {
//     return makeshot(STI::Engine::ShotType::Single);
// }

std::shared_ptr<STIPyShot>  STIPyServer::makeshot(const STI::Engine::ShotType& shotType)
{
    std::shared_ptr<STI::Engine::EventEngineScheduler> scheduler;
    std::shared_ptr<STI::Device::PersistenceManager> persistenceManager;
    std::shared_ptr<STI::Utils::FileServer> fileServer;

    STI::Engine::ShotConfig shotConfig;
    shotConfig.shotType = shotType;
    shotConfig.jobSourceID.machine = getHostname();
    shotConfig.jobSourceID.user = getUserName();

    if (libDevice == 0) return 0;
    
    bool success = libDevice->getFileServer(fileServer);

    auto stackTraceData = std::make_shared<StackTraceData>(libDevice->getID(), fileServer);
    auto eventGroup = std::make_shared<STI::Engine::RawEventGroup>("", "", stackTraceData);
    
    std::shared_ptr<STI::Engine::Shot> shot;

    if (getScheduler(scheduler)) {
        shot = scheduler->createShot(shotConfig, eventGroup);
    }
    else {
        throw py::value_error("Failed to get EventEngineScheduler to create shot.");
    }
    auto pyShot = std::make_shared<STIPyShot>(shot);
    return pyShot;
}

std::shared_ptr<STIPyShot> STIPyServer::makeshot(const std::function<void(void)>& func, const STI::Engine::ShotType& shotType)
{
    auto shot = makeshot(shotType);
    auto stipy = STI::Python::STIPyGlobal::getInstance();

    if (stipy != 0) {
        stipy->makeShot(shot, func);        
    }

    // const STI::Engine::ShotConfig& sc = shot->getShotConfig();
    // STI::Engine::ShotConfig& sc2 = const_cast<STI::Engine::ShotConfig&>(sc);

    // auto& files = shot->group()->getStackTraceData()->getTimingFiles();
    // if (!files.empty()) {
    //     sc2.file = files[0].getFullFilename();
    // } else {
    //     sc2.file = "default.shot";
    // }

    return shot;
}

std::shared_ptr<STIPyShot> STIPyServer::makeshot(const std::function<void(void)>& func, const std::set<ParsedVar>& vars, const STI::Engine::ShotType& shotType)
{
    auto shot = makeshot(shotType);
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

std::shared_ptr<PyParseTicket> STIPyServer::parse(const std::shared_ptr<STIPyShot>& pyShot, const STI::Engine::SequenceID& sequenceID)
{
    std::shared_ptr<STI::Engine::EventEngineScheduler> scheduler;
    bool success = false;
    STI::Engine::ParseJobStatus parseJobStatus;

    if (getScheduler(scheduler) && pyShot != 0) {
        parseJobStatus = scheduler->parse(pyShot->getShot(), sequenceID);
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


STI::Engine::SequenceID STIPyServer::addSequence(const std::shared_ptr<STI::Engine::Sequence>& seq)
{
    std::shared_ptr<STI::Engine::EventEngineScheduler> scheduler;
    bool success = false;
    STI::Engine::AddSequenceStatus addSequenceStatus;

    if (getScheduler(scheduler) && seq != 0) {
        // STI::Engine::EngineJobSourceID source;
        addSequenceStatus = scheduler->addSequence(seq, seq->shotConfig.jobSourceID);
        success = true;
    }
    
    if (addSequenceStatus.status == STI::Engine::EngineJobStatus::Deferred) {
    }

    return addSequenceStatus.seqid;
}

void STIPyServer::closeSequence(const STI::Engine::SequenceID& seqid)
{
    std::shared_ptr<STI::Engine::EventEngineScheduler> scheduler;

    if (getScheduler(scheduler)) {
        scheduler->closeSequence(seqid);
    }
}

void STIPyServer::cancelSequence(const STI::Engine::SequenceID& seqid)
{
    std::shared_ptr<STI::Engine::EventEngineScheduler> scheduler;

    if (getScheduler(scheduler)) {
        scheduler->cancelSequence(seqid);
    }
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
            resultTicket = play(ticket->getParseID());            
        }
        else {
            if (libDevice == 0) return 0;

            STI::Engine::EngineJobSourceID source;
            
            source.machine = libDevice->getID().getAddress();
            source.user = getUserName();

            auto shotID = STI::Engine::ShotID::generateUniqueID(ticket->getParseID(), source);
            resultTicket = libDevice->makeResultTicket(shotID);
            resultTicket->cancel();
        }
    }

    return resultTicket;
}

std::shared_ptr<PyResultTicket> STIPyServer::play(const STI::Engine::ParseID& parseID)
{
    STI::Engine::PlayJobStatus playJobStatus;
    bool success = false;

    std::shared_ptr<STI::Engine::EventEngineScheduler> scheduler;

    if (getScheduler(scheduler)) {

        STI::Engine::EngineJobSourceID source;
        if (libDevice != 0) {
            // source.machine = libDevice->getID().getAddress();
            source.machine = getHostname();
            source.user = getUserName();
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

void STIPyServer::cancelJob(const STI::Engine::EngineJobID& jobID)
{
    std::shared_ptr<STI::Engine::EventEngineScheduler> scheduler;

    if (getScheduler(scheduler)) {
        scheduler->cancelJob(jobID);
    }
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

void STIPyServer::setHostname(const std::string& name)
{
    hostname = name;
}

std::string STIPyServer::getHostname() const
{
    return hostname;
}

std::shared_ptr<STI::Network::NetworkDeviceHub> STIPyServer::getDeviceHub() const
{
    return libDeviceHub;
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
