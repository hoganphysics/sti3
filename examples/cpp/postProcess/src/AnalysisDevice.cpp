
#include "AnalysisDevice.h"

#include <sti/engine/ShotResult.h>

#include <memory>

using STI::Utils::MixedValue;
using STI::Utils::MixedValueType;
using STI::Utils::MetaData;
using STI::Engine::ShotResult;


AnalysisDevice::AnalysisDevice(const STI::Utils::Configuration& config)
: STI::Device::LocalDevice(config)
{
	// A measurement (input) channel.  In this single-device example the same
	// device plays the shot and post-processes it, so it is its own shot owner and
	// no addPartner() is needed.  (A dedicated analysis device that analyzes shots
	// played by *other* devices declares each owner with addPartner() so the worker
	// can pull their ShotResult.)
	addInputChannel(0, MixedValueType::Number, "signal");

	// Register a post-processing target with a member-function callback.  The
	// callback returns a results MetaData that is broadcast in a
	// PostProcessingComplete device message.  If it throws, the failure is
	// reported in that message instead of crashing the worker.
	addPostProcessingTarget(
		"atom number",
		[this](const std::shared_ptr<ShotResult>& shotResult, const MetaData& options) {
			return fitAtomNumber(shotResult, options);
		},
		"Reduces a shot's measurement data to an atom number.");

	// A target can also be a self-contained lambda.
	addPostProcessingTarget(
		"echo model",
		[](const std::shared_ptr<ShotResult>&, const MetaData& options) {
			MetaData results;
			results.addMetaData("model", options.getMetaData("model"));
			return results;
		},
		"Echoes the requested model name back in the results.");
}

MetaData AnalysisDevice::fitAtomNumber(const std::shared_ptr<ShotResult>& shotResult, const MetaData& options)
{
	// The worker has already pulled this shot's ShotResult from the owning device,
	// so a target reduces it directly -- no PersistenceManager lookup needed.  A
	// real target would fit or reduce shotResult->measurements here.
	bool haveData = (shotResult != 0);

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
