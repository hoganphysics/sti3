#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <sti/device/ChannelManager.h>
#include <sti/device/DeviceID.h>
#include <sti/device/LocalChannel.h>
#include <sti/device/PersistenceManager.h>
#include <sti/engine/DeviceEventParser.h>
#include <sti/engine/EngineID.h>
#include <sti/engine/EngineJobSourceID.h>
#include <sti/engine/EngineParsingMessage.h>
#include <sti/engine/FullShotResult.h>
#include <sti/engine/Measurement.h>
#include <sti/engine/RawEvent.h>
#include <sti/engine/RawEventGroup.h>
#include <sti/engine/RawEventTarget.h>
#include <sti/engine/ResultsCollector.h>
#include <sti/engine/SequenceResult.h>
#include <sti/engine/ShotID.h>
#include <sti/engine/ShotResult.h>
#include <sti/engine/SynchronousEvent.h>
#include <sti/utils/FileHolder.h>
#include <sti/utils/FileServer.h>
#include <sti/utils/MixedValue.h>
#include <sti/utils/VirtualFileHolder.h>
#include <sti/utils/VirtualFileServer.h>

#include "EventEngineParser.h"

#include <algorithm>
#include <memory>
#include <set>
#include <string>
#include <vector>

using STI::Device::Channel;
using STI::Device::ChannelManager;
using STI::Device::ChannelType;
using STI::Device::DeviceID;
using STI::Device::LocalChannel;
using STI::Device::PersistenceManager;
using STI::Engine::DeviceEventParser;
using STI::Engine::EngineID;
using STI::Engine::EngineJobSourceID;
using STI::Engine::EngineJobStatus;
using STI::Engine::EngineParsingMessage;
using STI::Engine::EventEngineParser;
using STI::Engine::FullShotResult;
using STI::Engine::MeasurementMap;
using STI::Engine::ParseID;
using STI::Engine::ParseResult;
using STI::Engine::RawEventGroup;
using STI::Engine::RawEventMap;
using STI::Engine::RawEventTarget;
using STI::Engine::RawEventType;
using STI::Engine::ResultsCollector;
using STI::Engine::ResultsCollectorFactory;
using STI::Engine::SequenceEntryID;
using STI::Engine::SequenceID;
using STI::Engine::SequenceResult;
using STI::Engine::ShotID;
using STI::Engine::ShotResult;
using STI::Engine::ShotResultRecord;
using STI::Engine::SynchronousEventVector;
using STI::Utils::FileHolder;
using STI::Utils::FileHolderFactory;
using STI::Utils::FileID;
using STI::Utils::FileServer;
using STI::Utils::MixedValue;
using STI::Utils::MixedValueType;
using STI::Utils::VirtualFileHolder;
using STI::Utils::VirtualFileServer;
using STI::Utils::VirtualFileServerFactory;

namespace {

class SingleChannelManager : public ChannelManager
{
public:
	SingleChannelManager(unsigned short channelNumber,
		ChannelType channelType,
		MixedValueType inputType,
		MixedValueType outputType)
		: channel(std::make_shared<LocalChannel>(
			channelNumber, channelType, inputType, outputType, "typed output"))
	{
	}

	void getChannels(std::vector<std::shared_ptr<Channel>>& channels) override
	{
		channels = { channel };
	}

	bool getChannel(short channelNumber, std::shared_ptr<Channel>& out) override
	{
		if (channelNumber == channel->getChannelNumber()) {
			out = channel;
			return true;
		}
		return false;
	}

	bool writeChannel(short, const MixedValue&) override { return false; }
	bool readChannel(short, const MixedValue&, MixedValue&) override { return false; }
	void stop() override {}

private:
	std::shared_ptr<LocalChannel> channel;
};

class NullPersistenceManager : public PersistenceManager
{
public:
	bool findShot(const ShotID&) override { return false; }
	bool getParseResult(const ParseID&, std::shared_ptr<ParseResult>&) override { return false; }
	bool getShotResult(const ShotID&, std::shared_ptr<ShotResult>&) override { return false; }
	bool getSequenceResult(const SequenceID&, std::shared_ptr<SequenceResult>&) override { return false; }
	bool saveShot(const ShotID&, const std::shared_ptr<FullShotResult>&, bool) override { return false; }
	ShotResultRecord transferResults(const std::shared_ptr<ResultsCollector>&) override { return ShotResultRecord(); }
	void setResultsCollectorFactory(const std::shared_ptr<ResultsCollectorFactory>&) override {}
	bool getMeasurements(const ShotID&, std::shared_ptr<MeasurementMap>&) override { return false; }
	void setFileHolderFactory(const std::shared_ptr<FileHolderFactory>&) override {}
	void setVirtualFileServerFactory(const std::shared_ptr<VirtualFileServerFactory>&) override {}
	void setFileServer(const std::shared_ptr<FileServer>& server) override { fileServer = server; }
	bool getFileServer(std::shared_ptr<FileServer>& server) override
	{
		server = fileServer;
		return server != nullptr;
	}

	std::shared_ptr<VirtualFileServer> makeVirtualFileServer() override
	{
		return std::make_shared<VirtualFileServer>();
	}

	void addSequence(const std::shared_ptr<SequenceResult>&) override {}
	bool updateSequence(const SequenceEntryID&, const ShotID&, const EngineJobStatus&, bool) override { return false; }
	bool saveSequence(const std::shared_ptr<SequenceResult>&, bool) override { return false; }

	std::shared_ptr<FileHolder> makeFileHolder(const std::string&, const std::string&) override { return {}; }
	std::shared_ptr<FileHolder> makeVirtualFileHolder(const FileID&) override { return {}; }
	std::shared_ptr<FileHolder> makeVirtualFileHolder(const std::shared_ptr<VirtualFileHolder>&) override { return {}; }

private:
	std::shared_ptr<FileServer> fileServer;
};

class CapturingDeviceEventParser : public DeviceEventParser
{
public:
	explicit CapturingDeviceEventParser(const DeviceID& deviceID)
		: deviceID(deviceID)
	{
	}

	bool isEventTarget(const DeviceID& id) override
	{
		return id == deviceID;
	}

	void getEventTargets(std::set<DeviceID>& targetIDs) override
	{
		targetIDs.insert(deviceID);
	}

	double getMinimumEventSpacing() override { return 1.0; }
	double getMinimumEventStartTime() override { return 0.0; }

	void parseEvents(const RawEventMap& events, SynchronousEventVector&) override
	{
		capturedEvents = events;
		parseCalls++;
	}

	RawEventMap capturedEvents;
	unsigned parseCalls = 0;

private:
	DeviceID deviceID;
};

ParseID makeParseID()
{
	return ParseID::generateUniqueID(EngineJobSourceID("eventengine-value-type-test", "localhost"));
}

bool hasParsingMessageNamed(const std::vector<EngineParsingMessage>& messages, const std::string& name)
{
	return std::any_of(messages.begin(), messages.end(), [&](const auto& message) {
		return message.getName() == name;
	});
}

} // namespace

TEST_CASE("EventEngineParser: Double channels accept integer event values as doubles", "[eventengine][mixedvalue]")
{
	constexpr unsigned short channelNumber = 3;
	const DeviceID deviceID("ValueTypeDevice", "127.0.0.1", 1);

	auto channelManager = std::make_shared<SingleChannelManager>(
		channelNumber, ChannelType::Output, MixedValueType::Empty, MixedValueType::Double);
	auto persistenceManager = std::make_shared<NullPersistenceManager>();
	CapturingDeviceEventParser deviceParser(deviceID);
	EventEngineParser parser(EngineID(0), deviceID, channelManager, persistenceManager, &deviceParser);

	RawEventGroup group("root", "");
	group.addEvent(RawEventTarget(deviceID, channelNumber), 10.0, MixedValue(5), RawEventType::Play);

	SynchronousEventVector synchedEvents;
	REQUIRE(parser.parse(group, synchedEvents, makeParseID()));
	REQUIRE(deviceParser.parseCalls == 1);

	auto eventIt = deviceParser.capturedEvents.find(10.0);
	REQUIRE(eventIt != deviceParser.capturedEvents.end());
	REQUIRE(eventIt->second.size() == 1);

	const auto& normalizedEvent = eventIt->second.front();
	CHECK(normalizedEvent.value().getType() == MixedValueType::Double);
	CHECK(normalizedEvent.value().getDouble() == Catch::Approx(5.0));

	auto sourceEvents = group.getEvents();
	REQUIRE(sourceEvents != nullptr);
	REQUIRE(sourceEvents->size() == 1);
	CHECK(sourceEvents->front().value().getType() == MixedValueType::Int);
	CHECK(sourceEvents->front().value().getInt() == 5);
}

TEST_CASE("EventEngineParser: numeric widening stays one-way", "[eventengine][mixedvalue]")
{
	constexpr unsigned short channelNumber = 3;
	const DeviceID deviceID("ValueTypeDevice", "127.0.0.1", 1);

	SECTION("Int channels reject double event values")
	{
		auto channelManager = std::make_shared<SingleChannelManager>(
			channelNumber, ChannelType::Output, MixedValueType::Empty, MixedValueType::Int);
		auto persistenceManager = std::make_shared<NullPersistenceManager>();
		CapturingDeviceEventParser deviceParser(deviceID);
		EventEngineParser parser(EngineID(0), deviceID, channelManager, persistenceManager, &deviceParser);

		RawEventGroup group("root", "");
		group.addEvent(RawEventTarget(deviceID, channelNumber), 10.0, MixedValue(5.0), RawEventType::Play);

		SynchronousEventVector synchedEvents;
		CHECK_FALSE(parser.parse(group, synchedEvents, makeParseID()));
		CHECK(deviceParser.parseCalls == 0);
		CHECK(hasParsingMessageNamed(parser.getParsingMessages(), "Incorrect Output Type"));
	}

	SECTION("Double channels reject boolean event values")
	{
		auto channelManager = std::make_shared<SingleChannelManager>(
			channelNumber, ChannelType::Output, MixedValueType::Empty, MixedValueType::Double);
		auto persistenceManager = std::make_shared<NullPersistenceManager>();
		CapturingDeviceEventParser deviceParser(deviceID);
		EventEngineParser parser(EngineID(0), deviceID, channelManager, persistenceManager, &deviceParser);

		RawEventGroup group("root", "");
		group.addEvent(RawEventTarget(deviceID, channelNumber), 10.0, MixedValue(true), RawEventType::Play);

		SynchronousEventVector synchedEvents;
		CHECK_FALSE(parser.parse(group, synchedEvents, makeParseID()));
		CHECK(deviceParser.parseCalls == 0);
		CHECK(hasParsingMessageNamed(parser.getParsingMessages(), "Incorrect Output Type"));
	}
}
