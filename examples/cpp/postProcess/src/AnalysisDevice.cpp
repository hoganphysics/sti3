
#include "AnalysisDevice.h"

#include <sti/device/PostProcessingManager.h>
#include <sti/device/PersistenceManager.h>
#include <sti/engine/ShotResult.h>

#include <memory>

using STI::Utils::MixedValue;
using STI::Utils::MixedValueType;
using STI::Utils::MetaData;
using STI::Engine::ShotID;


AnalysisDevice::AnalysisDevice(const STI::Utils::Configuration& config)
: STI::Device::LocalDevice(config)
{
	// A measurement (input) channel.  In this single-device example the same
	// device plays the shot and post-processes it, so its own PersistenceManager
	// holds the shot data the targets analyze.
	addInputChannel(0, MixedValueType::Number, "signal");

	// Register a post-processing target with a member-function callback.  The
	// callback returns a results MetaData that is broadcast in a
	// PostProcessingComplete device message.  If it throws, the failure is
	// reported in that message instead of crashing the worker.
	addPostProcessingTarget(
		"atom number",
		[this](const ShotID& shotID, const MetaData& options) {
			return fitAtomNumber(shotID, options);
		},
		"Reduces a shot's measurement data to an atom number.");

	// A target can also be a self-contained lambda.
	addPostProcessingTarget(
		"echo model",
		[](const ShotID& shotID, const MetaData& options) {
			MetaData results;
			results.addMetaData("model", options.getMetaData("model"));
			return results;
		},
		"Echoes the requested model name back in the results.");
}

MetaData AnalysisDevice::fitAtomNumber(const ShotID& shotID, const MetaData& options)
{
	// Pull this shot's data by ShotID.  The owning device's PersistenceManager
	// holds the result; dispatch happens after the result is persisted, so the
	// lookup succeeds.  A real target would fit or reduce the data here.
	bool haveData = false;
	std::shared_ptr<STI::Device::PersistenceManager> persistence;
	if (getPersistenceManager(persistence) && persistence != 0) {
		std::shared_ptr<STI::Engine::ShotResult> result;
		haveData = persistence->getShotResult(shotID, result) && result != 0;
	}

	MetaData results;
	results.addMetaData("haveShotData", MixedValue(haveData));
	results.addMetaData("N", MixedValue(1.0e6));
	return results;
}

bool AnalysisDevice::writeChannel(short channel, const STI::Utils::MixedValue& value)
{
	return false;	//this device has no output channels
}

bool AnalysisDevice::readChannel(short channel, const STI::Utils::MixedValue& value, STI::Utils::MixedValue& data)
{
	if (channel == 0) {
		data.setValue(23.4);	//simulated measurement
		return true;
	}
	return false;
}
