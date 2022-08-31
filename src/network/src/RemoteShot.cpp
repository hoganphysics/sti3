
#include "RemoteShot.h"

#include "Convert_EventEngine.h"
#include <sti/engine/RawEvent.h>
#include "ParseResult.h"

#include "Convert_ShotResult.h"
#include "Convert_RawEventGroup.h"

#include <memory>
#include <vector>

#include <iostream>

using STI::Network::RemoteShot;
using STI::Engine::ShotConfig;


RemoteShot::RemoteShot(const STI::Engine::ShotConfig& shotConfig, ::STI::TNetwork::TShotCallback_ptr shotCallback)
: STI::TNetwork::TReferenceHolder<STI::TNetwork::TShotCallback>(shotCallback, shotMutex), shotConfig(shotConfig)
{
	std::unique_lock<std::mutex> shotLock(shotMutex);

	refreshRequired = true;
}

RemoteShot::~RemoteShot()
{
	disable();
}

// void RemoteShot::setTimingFiles(const std::vector<std::shared_ptr<STI::Utils::FileHolder>>& files)
// {
// 	std::unique_lock<std::mutex> shotLock(shotMutex);
// 	timingFiles = files;
// }

// void RemoteShot::setFilenames(const std::vector<std::string>& filenames)
// {
// 	std::unique_lock<std::mutex> shotLock(shotMutex);
// 	timingFileNames = filenames;
// }

// void RemoteShot::setFunctionNames(const std::vector<std::string>& functions)
// {
// 	std::unique_lock<std::mutex> shotLock(shotMutex);
// 	functionNames = functions;
// }

bool RemoteShot::getTShotReference(STI::TNetwork::TShotCallback_ptr& tShotCallback)
{
	std::unique_lock<std::mutex> shotLock(shotMutex);

	if (isDisabled()) return false;

	tShotCallback = STI::TNetwork::TShotCallback::_duplicate(getTRef());

	return !CORBA::is_nil(tShotCallback);
}

const ShotConfig& RemoteShot::getShotConfig() const
{
	return shotConfig;
}

// void RemoteShot::getParseResult(std::shared_ptr<STI::Engine::ParseResult>& pResult)
// {
// 	std::unique_lock<std::mutex> shotLock(shotMutex);
// 	pResult = parseResult;
// }

// void RemoteShot::setParseResult(const std::shared_ptr<STI::Engine::ParseResult>& pResult)
// {
// 	// To do: call to servant? Maybe do nothing.
// 	// std::unique_lock<std::mutex> shotLock(shotMutex);
// 	// parseResult = pResult;
// }

// void RemoteShot::getEvents(std::shared_ptr<std::vector<STI::Engine::RawEvent>>& events)
// {
// 	refresh();

// 	std::unique_lock<std::mutex> shotLock(shotMutex);
// 	events = storedEvents;
// }


void RemoteShot::getBaseEventGroup(std::shared_ptr<STI::Engine::RawEventGroup>& baseGroup)
{
	refresh();

	std::unique_lock<std::mutex> shotLock(shotMutex);
	baseGroup = baseEventGroup;
}

void RemoteShot::refresh()
{
	std::unique_lock<std::mutex> shotLock(shotMutex);

	if (refreshRequired) {
		bool success = true;
		
		success &= refreshEvents();
		// success &= refreshParseResult();
		// success &= refreshGroups();
		// success &= refreshVars();
		// success &= refreshTags();

		refreshRequired = !success;		
	}

	//once events have been received, release remote reference
	if (!refreshRequired) {
		disable(shotLock);
	}
}

bool RemoteShot::refreshEvents()
{
	//has lock
	bool success = false;

	if (isDisabled()) return false;

	baseEventGroup = std::make_shared<STI::Engine::RawEventGroup>();

	STI::TNetwork::TRawEventGroup_var tEventGroup;

	try {

        getTRef()->getBaseEventGroup(tEventGroup); 	//remote call

		success = true;

		convert<STI::TNetwork::TRawEventGroup, std::shared_ptr<STI::Engine::RawEventGroup>>(tEventGroup, baseEventGroup);
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

// bool RemoteShot::refreshParseResult()
// {
// 	//has lock
// 	bool success = false;

// 	if (isDisabled()) return false;

// 	parseResult = std::make_shared<STI::Engine::ParseResult>();

// 	STI::TNetwork::TParseResult_var tParseResult;

// 	try {

//         getTRef()->getParseResult(tParseResult); 	//remote call

// 		// refreshRequired = false;
// 		success = true;

// 		if (storedEvents != 0) {
// 			convert<STI::TNetwork::TParseResult, STI::Engine::ParseResult>(tParseResult, *parseResult);
// 		}
// 	}
// 	catch (CORBA::TRANSIENT&) {
// 	}
// 	catch (CORBA::SystemException&) {
// 	}
// 	catch (CORBA::Exception&)
// 	{
// 	}

// 	return success;
// }

// std::vector<std::shared_ptr<STI::Utils::FileHolder>> RemoteShot::getTimingFiles() const
// {
// 	std::unique_lock<std::mutex> shotLock(shotMutex);
	
// 	if (parseResult != 0) {
// 		return parseResult->timingFiles;
// 	}
// 	std::vector<std::shared_ptr<STI::Utils::FileHolder>> files;
// 	return files;
// }


// std::vector<std::string> RemoteShot::getTimingFileNames() const
// {
// 	std::unique_lock<std::mutex> shotLock(shotMutex);
// 	if (parseResult != 0) {
// 		return parseResult->timingFileNames;
// 	}
// 	std::vector<std::string> filenames;
// 	return filenames;
// }

// std::vector<std::string> RemoteShot::getFunctionNames() const
// {
// 	std::unique_lock<std::mutex> shotLock(shotMutex);
// 	if (parseResult != 0) {
// 		return parseResult->functionNames;
// 	}
// 	std::vector<std::string> functions;
// 	return functions;
// }


// // std::vector<STI::Engine::RawEventGroup> RemoteShot::getGroups()
// std::vector<STI::Engine::RawEventGroup> RemoteShot::getGroups()
// {
// 	std::unique_lock<std::mutex> shotLock(shotMutex);
// 	if (parseResult != 0) {
// 		return parseResult->eventGroups;
// 	}
// 	std::vector<STI::Engine::RawEventGroup> groups;
// 	return groups;
// }

// std::vector<STI::Engine::ParsedVar> RemoteShot::getParsedVars()
// {
// 	std::unique_lock<std::mutex> shotLock(shotMutex);
// 	if (parseResult != 0) {
// 		return parseResult->parsedVars;
// 	}
// 	std::vector<STI::Engine::ParsedVar> vars;
// 	return vars;
// }

// std::vector<STI::Engine::ParsedTag> RemoteShot::getParsedTags()
// {
// 	std::unique_lock<std::mutex> shotLock(shotMutex);
// 	if (parseResult != 0) {
// 		return parseResult->parsedTags;
// 	}
// 	std::vector<STI::Engine::ParsedTag> tags;
// 	return tags;
// }


// bool RemoteShot::refreshGroups()
// {
// 	if (isDisabled()) return;

// 	bool success = false;

// 	std::vector<STI::Engine::RawEventGroup> groups;
// 	STI::TNetwork::TRawEventGroupSeq_var tGroups;

// 	try {

//         getTRef()->getGroups(tGroups); 	//remote call
// 		success = true;

// 		convert<STI::TNetwork::TRawEventGroup, STI::Engine::RawEventGroup>(tGroups, groups);

// 		parsedGroups.set(groups);
// 	}
// 	catch (CORBA::TRANSIENT&) {
// 	}
// 	catch (CORBA::SystemException&) {
// 	}
// 	catch (CORBA::Exception&)
// 	{
// 	}
// }

// bool RemoteShot::refreshVars()
// {
// 	if (isDisabled()) return;

// 	bool success = false;

// 	std::vector<STI::Engine::ParsedVar> vars;
// 	STI::TNetwork::TParsedVarSeq_var tVars;

// 	try {

//         getTRef()->getVars(tVars); 	//remote call
// 		success = true;

// 		convert<STI::TNetwork::TRawEventGroup, STI::Engine::RawEventGroup>(tVars, vars);

// 		parsedVars.set(vars);
// 	}
// 	catch (CORBA::TRANSIENT&) {
// 	}
// 	catch (CORBA::SystemException&) {
// 	}
// 	catch (CORBA::Exception&)
// 	{
// 	}
// }

// bool RemoteShot::refreshTags()
// {
// 	if (isDisabled()) return;

// 	bool success = false;
	
// 	std::vector<STI::Engine::ParsedTag> tags;
// 	STI::TNetwork::TParsedTagSeq_var tTags;

// 	try {

//         getTRef()->getTags(tTags); 	//remote call
// 		success = true;

// 		convert<STI::TNetwork::TRawEventGroup, STI::Engine::RawEventGroup>(tTags, tags);

// 		parsedTags.set(tags);
// 	}
// 	catch (CORBA::TRANSIENT&) {
// 	}
// 	catch (CORBA::SystemException&) {
// 	}
// 	catch (CORBA::Exception&)
// 	{
// 	}
// }

