#include <catch2/catch_test_macros.hpp>

#include "NetworkConvert.h"
#include "convert/Convert_Channel.h"
#include "convert/Convert_DeviceMessage.h"
#include "convert/Convert_EventEngine.h"
#include "convert/Convert_File.h"
#include "convert/Convert_Log.h"
#include "convert/Convert_Monitor.h"
#include "convert/Convert_Profile.h"
#include "convert/Convert_SequenceResult.h"
#include "convert/Convert_ShotResult.h"
#include "convert/Convert_Task.h"

#include <sti/device/DeviceMessageType.h>
#include <sti/device/LogFile.h>
#include <sti/device/LogRecord.h>
#include <sti/device/Monitor.h>
#include <sti/device/Profile.h>
#include <sti/engine/EngineJobID.h>
#include <sti/engine/EngineJobStatus.h>
#include <sti/engine/EngineParsingMessage.h>
#include <sti/engine/EnginePlayingMessage.h>
#include <sti/engine/EngineState.h>
#include <sti/engine/EventEngineJobList.h>
#include <sti/engine/Sequence.h>
#include <sti/engine/ShotConfig.h>
#include <sti/engine/ShotResult.h>
#include <sti/engine/ShotResultRecord.h>
#include <sti/fwd/Channel_fwd.h>
#include <sti/fwd/MixedValue_fwd.h>
#include <sti/fwd/RawEvent_fwd.h>
#include <sti/utils/FileServer.h>
#include <sti/utils/Task.h>

namespace
{

template<typename StiType, typename CorbaType>
void checkEnumRoundTrip(StiType stiValue, CorbaType corbaValue)
{
    CHECK(STI::Network::convert<StiType, CorbaType>(stiValue) == corbaValue);
    CHECK(STI::Network::convert<CorbaType, StiType>(corbaValue) == stiValue);
}

} // namespace

TEST_CASE("NetworkConvert: utility and device enum mappings round trip")
{
    checkEnumRoundTrip(STI::Utils::MixedValueType::Empty, STI::TNetwork::TMixedValueType::MixedValueEmpty);
    checkEnumRoundTrip(STI::Utils::MixedValueType::Boolean, STI::TNetwork::TMixedValueType::MixedValueBoolean);
    checkEnumRoundTrip(STI::Utils::MixedValueType::Int, STI::TNetwork::TMixedValueType::MixedValueInt);
    checkEnumRoundTrip(STI::Utils::MixedValueType::Double, STI::TNetwork::TMixedValueType::MixedValueDouble);
    checkEnumRoundTrip(STI::Utils::MixedValueType::String, STI::TNetwork::TMixedValueType::MixedValueString);
    checkEnumRoundTrip(STI::Utils::MixedValueType::Vector, STI::TNetwork::TMixedValueType::MixedValueVector);
    checkEnumRoundTrip(STI::Utils::MixedValueType::VectorInt, STI::TNetwork::TMixedValueType::MixedValueVectorInt);
    checkEnumRoundTrip(STI::Utils::MixedValueType::File, STI::TNetwork::TMixedValueType::MixedValueFile);
    checkEnumRoundTrip(STI::Utils::MixedValueType::Image, STI::TNetwork::TMixedValueType::MixedValueImage);
    checkEnumRoundTrip(STI::Utils::MixedValueType::Number, STI::TNetwork::TMixedValueType::MixedValueNumber);
    checkEnumRoundTrip(STI::Utils::MixedValueType::Any, STI::TNetwork::TMixedValueType::MixedValueAny);

    checkEnumRoundTrip(STI::Utils::FileTransferType::Binary, STI::TNetwork::TFileTransferType::FileTransferBinary);
    checkEnumRoundTrip(STI::Utils::FileTransferType::String, STI::TNetwork::TFileTransferType::FileTransferString);

    checkEnumRoundTrip(STI::Utils::TaskStatus::Active, STI::TNetwork::TTaskStatus::TaskActive);
    checkEnumRoundTrip(STI::Utils::TaskStatus::Inactive, STI::TNetwork::TTaskStatus::TaskInactive);
    checkEnumRoundTrip(STI::Utils::TaskStatus::Missing, STI::TNetwork::TTaskStatus::TaskMissing);

    checkEnumRoundTrip(STI::Device::ChannelType::Output, STI::TNetwork::TChannelType::TChannelOutput);
    checkEnumRoundTrip(STI::Device::ChannelType::Input, STI::TNetwork::TChannelType::TChannelInput);

    checkEnumRoundTrip(STI::Device::MonitorStatus::Active, STI::TNetwork::TMonitorStatus::MonitorActive);
    checkEnumRoundTrip(STI::Device::MonitorStatus::Inactive, STI::TNetwork::TMonitorStatus::MonitorInactive);
    checkEnumRoundTrip(STI::Device::MonitorStatus::Missing, STI::TNetwork::TMonitorStatus::MonitorMissing);

    checkEnumRoundTrip(STI::Device::ProfileType::Attribute, STI::TNetwork::TProfileType::ProfileAttribute);
    checkEnumRoundTrip(STI::Device::ProfileType::Channel, STI::TNetwork::TProfileType::ProfileChannel);
    checkEnumRoundTrip(STI::Device::ProfileType::All, STI::TNetwork::TProfileType::ProfileAll);
}

TEST_CASE("NetworkConvert: device message enum mappings round trip")
{
    checkEnumRoundTrip(STI::Device::DeviceMessageType::Refresh, STI::TNetwork::TDeviceMessageType::MessageRefresh);
    checkEnumRoundTrip(
        STI::Device::DeviceMessageType::CollectionUpdate,
        STI::TNetwork::TDeviceMessageType::MessageCollectionUpdate);
    checkEnumRoundTrip(
        STI::Device::DeviceMessageType::ChannelUpdate,
        STI::TNetwork::TDeviceMessageType::MessageChannelUpdate);
    checkEnumRoundTrip(
        STI::Device::DeviceMessageType::ChannelsRefresh,
        STI::TNetwork::TDeviceMessageType::MessageChannelsRefresh);
    checkEnumRoundTrip(
        STI::Device::DeviceMessageType::AttributeUpdate,
        STI::TNetwork::TDeviceMessageType::MessageAttributeUpdate);
    checkEnumRoundTrip(
        STI::Device::DeviceMessageType::AttributesRefresh,
        STI::TNetwork::TDeviceMessageType::MessageAttributesRefresh);
    checkEnumRoundTrip(
        STI::Device::DeviceMessageType::MonitorUpdate,
        STI::TNetwork::TDeviceMessageType::MessageMonitorUpdate);
    checkEnumRoundTrip(
        STI::Device::DeviceMessageType::MonitorStatusUpdate,
        STI::TNetwork::TDeviceMessageType::MessageMonitorStatusUpdate);
    checkEnumRoundTrip(
        STI::Device::DeviceMessageType::TaskUpdate,
        STI::TNetwork::TDeviceMessageType::MessageTaskUpdate);
    checkEnumRoundTrip(
        STI::Device::DeviceMessageType::EngineScheduler,
        STI::TNetwork::TDeviceMessageType::MessageEngineScheduler);
    checkEnumRoundTrip(
        STI::Device::DeviceMessageType::EngineParser,
        STI::TNetwork::TDeviceMessageType::MessageEngineParser);
    checkEnumRoundTrip(
        STI::Device::DeviceMessageType::EngineStatus,
        STI::TNetwork::TDeviceMessageType::MessageEngineStatus);
    checkEnumRoundTrip(
        STI::Device::DeviceMessageType::EngineJobUpdate,
        STI::TNetwork::TDeviceMessageType::MessageEngineJobUpdate);
    checkEnumRoundTrip(STI::Device::DeviceMessageType::Unknown, STI::TNetwork::TDeviceMessageType::MessageUnknown);
}

TEST_CASE("NetworkConvert: log enum mappings round trip")
{
    checkEnumRoundTrip(STI::Device::LogRecordStatus::Unqueried, STI::TNetwork::TLogRecordStatus::LogRecordUnqueried);
    checkEnumRoundTrip(
        STI::Device::LogRecordStatus::LogsPresent,
        STI::TNetwork::TLogRecordStatus::LogRecordLogsPresent);
    checkEnumRoundTrip(STI::Device::LogRecordStatus::NoLogs, STI::TNetwork::TLogRecordStatus::LogRecordNoLogs);
    checkEnumRoundTrip(STI::Device::LogRecordStatus::Error, STI::TNetwork::TLogRecordStatus::LogRecordError);

    checkEnumRoundTrip(STI::Device::LogFile::LogFileType::FileID, STI::TNetwork::TLogFileType::LogFileFileID);
    checkEnumRoundTrip(
        STI::Device::LogFile::LogFileType::FileHolder,
        STI::TNetwork::TLogFileType::LogFileFileHolder);
    checkEnumRoundTrip(STI::Device::LogFile::LogFileType::String, STI::TNetwork::TLogFileType::LogFileString);
}

TEST_CASE("NetworkConvert: engine enum mappings round trip")
{
    checkEnumRoundTrip(STI::Engine::EngineState::Idle, STI::TNetwork::TEngineState::EngineIdle);
    checkEnumRoundTrip(STI::Engine::EngineState::Parsing, STI::TNetwork::TEngineState::EngineParsing);
    checkEnumRoundTrip(STI::Engine::EngineState::Parsed, STI::TNetwork::TEngineState::EngineParsed);
    checkEnumRoundTrip(STI::Engine::EngineState::PreparingPlay, STI::TNetwork::TEngineState::EnginePreparingPlay);
    checkEnumRoundTrip(STI::Engine::EngineState::PlayReady, STI::TNetwork::TEngineState::EnginePlayReady);
    checkEnumRoundTrip(
        STI::Engine::EngineState::WaitingForTrigger,
        STI::TNetwork::TEngineState::EngineWaitingForTrigger);
    checkEnumRoundTrip(STI::Engine::EngineState::Playing, STI::TNetwork::TEngineState::EnginePlaying);
    checkEnumRoundTrip(STI::Engine::EngineState::Paused, STI::TNetwork::TEngineState::EnginePaused);
    checkEnumRoundTrip(STI::Engine::EngineState::Unknown, STI::TNetwork::TEngineState::EngineUnknown);
    checkEnumRoundTrip(STI::Engine::EngineState::Missing, STI::TNetwork::TEngineState::EngineMissing);
    checkEnumRoundTrip(STI::Engine::EngineState::Error, STI::TNetwork::TEngineState::EngineError);

    checkEnumRoundTrip(STI::Engine::EventEngineJobType::Parse, STI::TNetwork::TEventEngineJobType::EngineJobParse);
    checkEnumRoundTrip(STI::Engine::EventEngineJobType::Play, STI::TNetwork::TEventEngineJobType::EngineJobPlay);
    checkEnumRoundTrip(
        STI::Engine::EventEngineJobType::Sequence,
        STI::TNetwork::TEventEngineJobType::EngineJobSequence);

    checkEnumRoundTrip(STI::Engine::EngineJobStatus::New, STI::TNetwork::TEngineJobStatus::JobNew);
    checkEnumRoundTrip(STI::Engine::EngineJobStatus::Running, STI::TNetwork::TEngineJobStatus::JobRunning);
    checkEnumRoundTrip(STI::Engine::EngineJobStatus::Completed, STI::TNetwork::TEngineJobStatus::JobCompleted);
    checkEnumRoundTrip(STI::Engine::EngineJobStatus::Canceled, STI::TNetwork::TEngineJobStatus::JobCanceled);
    checkEnumRoundTrip(STI::Engine::EngineJobStatus::NotFound, STI::TNetwork::TEngineJobStatus::JobNotFound);
    checkEnumRoundTrip(STI::Engine::EngineJobStatus::Archived, STI::TNetwork::TEngineJobStatus::JobArchived);
    checkEnumRoundTrip(STI::Engine::EngineJobStatus::Deferred, STI::TNetwork::TEngineJobStatus::JobDeferred);

    checkEnumRoundTrip(
        STI::Engine::EventEngineJobList::Queued,
        STI::TNetwork::TEventEngineJobList::EngineJobListQueued);
    checkEnumRoundTrip(
        STI::Engine::EventEngineJobList::Running,
        STI::TNetwork::TEventEngineJobList::EngineJobListRunning);
    checkEnumRoundTrip(
        STI::Engine::EventEngineJobList::Completed,
        STI::TNetwork::TEventEngineJobList::EngineJobListCompleted);
    checkEnumRoundTrip(
        STI::Engine::EventEngineJobList::Archived,
        STI::TNetwork::TEventEngineJobList::EngineJobListArchived);
}

TEST_CASE("NetworkConvert: event and result enum mappings round trip")
{
    checkEnumRoundTrip(STI::Engine::RawEventType::Play, STI::TNetwork::TRawEventType::RawEventPlay);
    checkEnumRoundTrip(STI::Engine::RawEventType::Measurement, STI::TNetwork::TRawEventType::RawEventMeasurement);
    checkEnumRoundTrip(STI::Engine::RawEventType::Waveform, STI::TNetwork::TRawEventType::RawEventWaveform);
    checkEnumRoundTrip(STI::Engine::RawEventType::Pause, STI::TNetwork::TRawEventType::RawEventPause);
    checkEnumRoundTrip(STI::Engine::RawEventType::Jump, STI::TNetwork::TRawEventType::RawEventJump);

    checkEnumRoundTrip(STI::Engine::ShotType::Single, STI::TNetwork::TShotType::ShotTypeSingle);
    checkEnumRoundTrip(STI::Engine::ShotType::Sequence, STI::TNetwork::TShotType::ShotTypeSequence);
    checkEnumRoundTrip(
        STI::Engine::ShotType::SingleUndocumented,
        STI::TNetwork::TShotType::ShotTypeSingleUndocumented);
    checkEnumRoundTrip(STI::Engine::ShotType::SequenceEntry, STI::TNetwork::TShotType::ShotTypeSequenceEntry);

    checkEnumRoundTrip(STI::Engine::RecordStatus::Unqueried, STI::TNetwork::TRecordStatus::TRecordUnqueried);
    checkEnumRoundTrip(STI::Engine::RecordStatus::Complete, STI::TNetwork::TRecordStatus::TRecordComplete);
    checkEnumRoundTrip(STI::Engine::RecordStatus::MissingDevice, STI::TNetwork::TRecordStatus::TRecordMissingDevice);
    checkEnumRoundTrip(
        STI::Engine::RecordStatus::MissingResults,
        STI::TNetwork::TRecordStatus::TRecordMissingResults);
    checkEnumRoundTrip(STI::Engine::RecordStatus::Error, STI::TNetwork::TRecordStatus::TRecordError);

    checkEnumRoundTrip(STI::Engine::SequenceType::Open, STI::TNetwork::TSequenceType::SequenceTypeOpen);
    checkEnumRoundTrip(STI::Engine::SequenceType::Closed, STI::TNetwork::TSequenceType::SequenceTypeClosed);
}

TEST_CASE("NetworkConvert: message severity and shot-result enum mappings round trip")
{
    checkEnumRoundTrip(STI::Engine::ParsingMessageType::Error, STI::TNetwork::TParsingMessageType::ParsingError);
    checkEnumRoundTrip(STI::Engine::ParsingMessageType::Warning, STI::TNetwork::TParsingMessageType::ParsingWarning);
    checkEnumRoundTrip(
        STI::Engine::ParsingMessageType::Information,
        STI::TNetwork::TParsingMessageType::ParsingInformation);

    checkEnumRoundTrip(STI::Engine::PlayingMessageType::Error, STI::TNetwork::TPlayingMessageType::PlayingError);
    checkEnumRoundTrip(STI::Engine::PlayingMessageType::Warning, STI::TNetwork::TPlayingMessageType::PlayingWarning);
    checkEnumRoundTrip(
        STI::Engine::PlayingMessageType::Information,
        STI::TNetwork::TPlayingMessageType::PlayingInformation);

    checkEnumRoundTrip(
        STI::Engine::ShotResultStatus::Success,
        STI::TNetwork::TShotResultStatus::TShotResultSuccess);
    checkEnumRoundTrip(
        STI::Engine::ShotResultStatus::CompletedWithErrors,
        STI::TNetwork::TShotResultStatus::TShotResultCompletedWithErrors);
    checkEnumRoundTrip(
        STI::Engine::ShotResultStatus::CanceledByUser,
        STI::TNetwork::TShotResultStatus::TShotResultCanceledByUser);
    checkEnumRoundTrip(
        STI::Engine::ShotResultStatus::AbortedByError,
        STI::TNetwork::TShotResultStatus::TShotResultAbortedByError);
    checkEnumRoundTrip(
        STI::Engine::ShotResultStatus::AbortedByTimeout,
        STI::TNetwork::TShotResultStatus::TShotResultAbortedByTimeout);
    checkEnumRoundTrip(
        STI::Engine::ShotResultStatus::Unknown,
        STI::TNetwork::TShotResultStatus::TShotResultUnknown);
}
