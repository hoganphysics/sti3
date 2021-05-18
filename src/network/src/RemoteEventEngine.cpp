
#include "RemoteEventEngine.h"
#include "Convert_EventEngine.h"
#include "RawEvent.h"

using STI::Network::RemoteEventEngine;

using STI::Engine::EventEngineJob;
using STI::TNetwork::TEventEngineJob;
using STI::Network::convert;
using STI::Engine::TriggerCallback;
using STI::TNetwork::TParseID;
using STI::Engine::ParseID;


RemoteEventEngine::RemoteEventEngine(::STI::TNetwork::TEventEngine_ptr engine)
	: _tEngine(STI::TNetwork::TEventEngine::_duplicate(engine))
{
}

RemoteEventEngine::~RemoteEventEngine()
{
}


void RemoteEventEngine::play(EventEngineJob& job)
{
	try {
		_tEngine->play(convert<EventEngineJob, TEventEngineJob>(job));
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}
}

void RemoteEventEngine::play(const STI::Engine::EngineJobID& jobID, const std::shared_ptr<TriggerCallback>& triggerCB, bool debug)
{
    triggerCallbackServant = std::make_shared<STI::TNetwork::TTriggerCallback_i>(triggerCB);

	try {
		_tEngine->playCB(
            convert<STI::Engine::EngineJobID, STI::TNetwork::TEngineJobID>(jobID),
            (*triggerCallbackServant)._this(),
            static_cast<CORBA::Boolean>(debug)
            );
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}
}


void RemoteEventEngine::trigger()
{
	try {
		_tEngine->trigger();
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}
}

void RemoteEventEngine::trigger(const STI::Device::DeviceID& target)
{
	try {
		_tEngine->triggerTarget(convert<STI::Device::DeviceID, STI::TNetwork::TDeviceID>(target));
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}
}


void RemoteEventEngine::stop()
{
	try {
		_tEngine->stop();
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}
}

void RemoteEventEngine::pause()
{
	try {
		_tEngine->pause();
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}
}

void RemoteEventEngine::unpause(bool retrigger)
{
	try {
		_tEngine->unpause(static_cast<CORBA::Boolean>(retrigger));
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}
}


STI::Device::DeviceID RemoteEventEngine::getDeviceID() const
{
	::STI::TNetwork::TDeviceID_var tDeviceID;

	bool success = false;

	try {
		tDeviceID = _tEngine->getDeviceID();
		success = true;
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}

	STI::Device::DeviceID deviceID;

	if(success) {
		deviceID = convert<::STI::TNetwork::TDeviceID, STI::Device::DeviceID>(tDeviceID);
	}

	return deviceID;
}


STI::Engine::EngineState RemoteEventEngine::getState() const
{
	::STI::TNetwork::TEngineState tState = ::STI::TNetwork::EngineUnknown;

	bool success = false;

	try {
		tState = _tEngine->getState();
		success = true;
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}

	STI::Engine::EngineState state;

	if(success) {
		convert<::STI::TNetwork::TEngineState, STI::Engine::EngineState>(tState, state);
	}

	return state;
}


// const STI::Engine::DeviceEventMap& RemoteEventEngine::getParsedEvents()
bool RemoteEventEngine::getParsedEvents(const STI::Engine::ParseID& parseID, STI::Engine::DeviceEventMap& parsedEvents)
{
	if (CORBA::is_nil(_tEngine)) return false;

	STI::TNetwork::TDeviceEventsSeq_var tEngineParsedEvents(new STI::TNetwork::TDeviceEventsSeq);

	bool success = false;

	try {
		success = _tEngine->getParsedEvents(convert<ParseID, TParseID>(parseID), tEngineParsedEvents);	//remote call

		convert<::STI::TNetwork::TDeviceEventsSeq, STI::Engine::DeviceEventMap>(tEngineParsedEvents, parsedEvents);
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}
	return success;
}

