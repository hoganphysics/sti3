#ifndef ANALYSISDEVICE_H
#define ANALYSISDEVICE_H

#include <sti/LocalDevice.h>

#include <sti/engine/ShotID.h>
#include <sti/utils/MetaData.h>


// A device that hosts post-processing targets.
//
// A post-processing target is a named analysis routine that runs after a shot
// finishes playing, on a background worker thread, without blocking the parsing
// or playback of later shots.  A common pattern is a small, dedicated analysis
// device whose only job is to host targets.
//
// A timing file requests post-processing against a target with postTarget() and
// postProcess() (no time argument -- a postProcess() request is not hard-timed).
class AnalysisDevice : public STI::Device::LocalDevice
{
public:

	AnalysisDevice(const STI::Utils::Configuration& config);

	//implementation of LocalDevice
	bool writeChannel(short channel, const STI::Utils::MixedValue& value);
	bool readChannel(short channel, const STI::Utils::MixedValue& value, STI::Utils::MixedValue& data);

	//post-processing callback: receives the completed shot's ID and the per-request
	//options, and returns a results MetaData that is broadcast on completion.
	STI::Utils::MetaData fitAtomNumber(const STI::Engine::ShotID& shotID, const STI::Utils::MetaData& options);

};


#endif
