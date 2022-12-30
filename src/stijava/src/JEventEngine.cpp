

#include "JEventEngine.h"
#include "EventEngine.h"
#include "JEventEngineJob.h"


using STI::Engine::JEventEngine;
using STI::Engine::EventEngine;
using STI::Engine::JEventEngineJob;


JEventEngine::JEventEngine(const std::shared_ptr<EventEngine>& engine)
: engine(engine)
{
}

JEventEngine::~JEventEngine()
{
}

void JEventEngine::play(const std::shared_ptr<JEventEngineJob>& job)
{
    if (job == 0) return;
            
    auto eej = job->eventEngineJob;
    
    if (eej != 0 && engine != 0) {
        engine->play(*eej);
    }
}


void JEventEngine::stop()
{
    if (engine != 0) {
        engine->stop();
    }
}

void JEventEngine::pause()
{
    if (engine != 0) {
        engine->pause();
    }
}

void JEventEngine::unpause(bool retrigger)
{
    if (engine != 0) {
        engine->unpause(retrigger);
    }
}


STI::Device::DeviceID JEventEngine::getDeviceID() const
{
    if (engine != 0) {
        return engine->getDeviceID();
    }

    STI::Device::DeviceID id;   //empty
    return id;
}

STI::Engine::EngineState JEventEngine::getState() const
{
    if (engine != 0) {
        return engine->getState();
    }

    STI::Engine::EngineState state = STI::Engine::EngineState::Unknown;
    return state;
}

