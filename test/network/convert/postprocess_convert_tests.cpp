#include <catch2/catch_test_macros.hpp>

#include "NetworkConvert.h"
#include "convert/Convert_RawEventGroup.h"
#include "convert/Convert_DeviceMessage.h"
#include "convert/Convert_PostProcessing.h"

#include <sti/device/PostProcessingManager.h>

#include <sti/engine/RawEventGroup.h>
#include <sti/engine/RawEventTarget.h>
#include <sti/engine/PostProcessRequest.h>
#include <sti/engine/PostProcessTarget.h>
#include <sti/engine/RawEventTargetDevice.h>
#include <sti/engine/ShotID.h>
#include <sti/device/DeviceID.h>
#include <sti/device/DeviceMessage.h>
#include <sti/device/DeviceTrace.h>
#include <sti/utils/MetaData.h>
#include <sti/utils/MixedValue.h>

#include <memory>

using STI::Device::DeviceID;
using STI::Device::DeviceMessage;
using STI::Device::DeviceTrace;
using STI::Device::PostProcessingCompleteMessage;
using STI::Device::PostProcessingStatus;
using STI::Engine::PostProcessRequest;
using STI::Engine::PostProcessTarget;
using STI::Engine::RawEventGroup;
using STI::Engine::RawEventTargetDevice;
using STI::Engine::ShotID;
using STI::Utils::MetaData;
using STI::Utils::MixedValue;
using STI::TNetwork::TRawEventGroup;
using STI::TNetwork::TAnyMessage;
using STI::TNetwork::TDeviceMessageType;


TEST_CASE("NetworkConvert: RawEventGroup post-processing side-list round trips", "[postprocessing][convert]")
{
    auto group = std::make_shared<RawEventGroup>("root", "");

    DeviceID analysisID("Analysis", "localhost", 2);

    MetaData options;
    options.addMetaData("fitType", MixedValue(std::string("gaussian")));

    PostProcessTarget target(RawEventTargetDevice(analysisID), "Blue MOT");
    group->addPostProcessRequest(PostProcessRequest(target, options));

    // An abstract, name-only target should survive too.
    PostProcessTarget abstractTarget("OtherAnalysis", "Red MOT");
    group->addPostProcessRequest(PostProcessRequest(abstractTarget, MetaData()));

    TRawEventGroup tGroup;
    REQUIRE(STI::Network::convert<std::shared_ptr<RawEventGroup>, TRawEventGroup>(group, tGroup));
    REQUIRE(tGroup.postProcessRequests.length() == 2);

    std::shared_ptr<RawEventGroup> restored;
    REQUIRE(STI::Network::convert<TRawEventGroup, std::shared_ptr<RawEventGroup>>(tGroup, restored));
    REQUIRE(restored != nullptr);

    const auto& requests = restored->postProcessRequests();
    REQUIRE(requests.size() == 2);

    CHECK(requests[0].target().name() == "Blue MOT");
    CHECK_FALSE(requests[0].target().isAbstract());
    CHECK(requests[0].target().device().deviceID() == analysisID);
    CHECK(requests[0].options().contains("fitType"));
    CHECK(requests[0].options().getMetaData("fitType").getString() == "gaussian");

    CHECK(requests[1].target().name() == "Red MOT");
    CHECK(requests[1].target().isAbstract());
    CHECK(requests[1].target().device().name() == "OtherAnalysis");
}

TEST_CASE("NetworkConvert: PostProcessingTargetInfo with option hints round trips", "[postprocessing][convert]")
{
    using STI::Device::PostProcessingOptionInfo;
    using STI::Device::PostProcessingTargetInfo;
    using STI::TNetwork::TPostProcessingTargetInfo;

    std::vector<PostProcessingTargetInfo> targets;

    PostProcessingTargetInfo fit;
    fit.name = "atom number";
    fit.description = "Counts atoms from an absorption image.";
    fit.options.push_back(PostProcessingOptionInfo{"roi", "region of interest"});
    fit.options.push_back(PostProcessingOptionInfo{"model", ""});   //description optional
    targets.push_back(fit);

    PostProcessingTargetInfo echo;   //no options
    echo.name = "echo";
    echo.description = "Echoes options.";
    targets.push_back(echo);

    //vector -> seq -> vector
    STI::TNetwork::TPostProcessingTargetInfoSeq tSeq;
    REQUIRE(STI::Network::convert<PostProcessingTargetInfo, TPostProcessingTargetInfo>(targets, tSeq));
    REQUIRE(tSeq.length() == 2);

    std::vector<PostProcessingTargetInfo> restored;
    REQUIRE(STI::Network::convert<TPostProcessingTargetInfo, PostProcessingTargetInfo>(tSeq, restored));
    REQUIRE(restored.size() == 2);

    CHECK(restored[0].name == "atom number");
    CHECK(restored[0].description == "Counts atoms from an absorption image.");
    REQUIRE(restored[0].options.size() == 2);
    CHECK(restored[0].options[0].name == "roi");
    CHECK(restored[0].options[0].description == "region of interest");
    CHECK(restored[0].options[1].name == "model");
    CHECK(restored[0].options[1].description.empty());

    CHECK(restored[1].name == "echo");
    CHECK(restored[1].options.empty());
}

TEST_CASE("NetworkConvert: PostProcessingCompleteMessage round trips through TAnyMessage", "[postprocessing][convert]")
{
    DeviceTrace trace(DeviceID("Owner", "localhost", 0));

    SECTION("success carries results")
    {
        auto message = std::make_shared<PostProcessingCompleteMessage>(trace);
        message->targetName = "fit";
        message->status = PostProcessingStatus::Success;
        message->results.addMetaData("amplitude", MixedValue(2.5));

        TAnyMessage tAny;
        REQUIRE(STI::Network::convert<std::shared_ptr<DeviceMessage>, TAnyMessage>(message, tAny));
        CHECK(tAny.type == TDeviceMessageType::MessagePostProcessingComplete);

        std::shared_ptr<DeviceMessage> restoredBase;
        REQUIRE(STI::Network::convert<TAnyMessage, std::shared_ptr<DeviceMessage>>(tAny, restoredBase));

        auto restored = std::dynamic_pointer_cast<PostProcessingCompleteMessage>(restoredBase);
        REQUIRE(restored != nullptr);
        CHECK(restored->targetName == "fit");
        CHECK(restored->status == PostProcessingStatus::Success);
        CHECK(restored->results.contains("amplitude"));
        CHECK(restored->results.getMetaData("amplitude").getDouble() == 2.5);
        CHECK(restored->errorMessage.empty());
    }

    SECTION("failure carries error message")
    {
        auto message = std::make_shared<PostProcessingCompleteMessage>(trace);
        message->targetName = "boom";
        message->status = PostProcessingStatus::Failed;
        message->errorMessage = "kaboom";

        TAnyMessage tAny;
        REQUIRE(STI::Network::convert<std::shared_ptr<DeviceMessage>, TAnyMessage>(message, tAny));

        std::shared_ptr<DeviceMessage> restoredBase;
        REQUIRE(STI::Network::convert<TAnyMessage, std::shared_ptr<DeviceMessage>>(tAny, restoredBase));

        auto restored = std::dynamic_pointer_cast<PostProcessingCompleteMessage>(restoredBase);
        REQUIRE(restored != nullptr);
        CHECK(restored->targetName == "boom");
        CHECK(restored->status == PostProcessingStatus::Failed);
        CHECK(restored->errorMessage == "kaboom");
    }
}
