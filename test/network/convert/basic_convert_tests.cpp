#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "NetworkConvert.h"
#include "convert/Convert_Attribute.h"
#include "convert/Convert_EventEngine.h"
#include "convert/Convert_File.h"

#include <sti/utils/FileID.h>
#include <sti/utils/MixedValue.h>
#include <sti/utils/TimeStamp.h>

#include <array>
#include <map>
#include <string>
#include <vector>

namespace
{

std::string toString(const CORBA::String_member& value)
{
    return STI::Network::convert<CORBA::String_member, std::string>(value);
}

STI::Utils::FileID makeFileID()
{
    STI::Utils::FileID fileID;
    fileID.filename = "result.dat";
    fileID.path = "/tmp/sti3/results";
    fileID.origin = "device-a";
    fileID.persistenceLocation = "local-cache";
    fileID.creationTime = STI::Utils::TimeStamp(2026, 5, 9, 14, 30, 12, 123, 456, 789);
    return fileID;
}

void checkFileID(const STI::Utils::FileID& actual, const STI::Utils::FileID& expected)
{
    CHECK(actual.filename == expected.filename);
    CHECK(actual.path == expected.path);
    CHECK(actual.origin == expected.origin);
    CHECK(actual.persistenceLocation == expected.persistenceLocation);
    CHECK(actual.creationTime == expected.creationTime);
}

} // namespace

TEST_CASE("NetworkConvert: strings and string sequences round trip")
{
    const std::string original = "scope/device channel";
    auto corbaString = STI::Network::convert<std::string, CORBA::String_member>(original);

    CHECK(toString(corbaString) == original);

    STI::TNetwork::TStringSeq tStrings;
    const std::vector<std::string> strings = {"alpha", "beta", "gamma"};

    REQUIRE(STI::Network::convert<std::vector<std::string>, STI::TNetwork::TStringSeq>(strings, tStrings));
    REQUIRE(tStrings.length() == strings.size());
    CHECK(std::string(tStrings[0]._NP_ref()) == "alpha");
    CHECK(std::string(tStrings[1]._NP_ref()) == "beta");
    CHECK(std::string(tStrings[2]._NP_ref()) == "gamma");

    std::vector<std::string> roundTrip;
    REQUIRE(STI::Network::convert<STI::TNetwork::TStringSeq, std::vector<std::string>>(tStrings, roundTrip));
    CHECK(roundTrip == strings);
}

TEST_CASE("NetworkConvert: string maps round trip through CORBA tuple sequences")
{
    const std::map<std::string, std::string> values = {
        {"mode", "scan"},
        {"owner", "lab-a"},
        {"priority", "high"},
    };

    STI::TNetwork::TStringPairSeq tPairs;
    REQUIRE(STI::Network::convert<std::map<std::string, std::string>, STI::TNetwork::TStringPairSeq>(values, tPairs));
    REQUIRE(tPairs.length() == values.size());

    std::map<std::string, std::string> roundTrip;
    REQUIRE(STI::Network::convert<STI::TNetwork::TStringPairSeq, std::map<std::string, std::string>>(tPairs, roundTrip));
    CHECK(roundTrip == values);
}

TEST_CASE("NetworkConvert: attribute maps round trip through CORBA tuple sequences")
{
    const std::map<std::string, std::string> attributes = {
        {"bias", "2.4"},
        {"enabled", "true"},
        {"units", "V"},
    };

    STI::TNetwork::TAttributeTupleSeq tAttributes;
    REQUIRE(STI::Network::convert<std::map<std::string, std::string>, STI::TNetwork::TAttributeTupleSeq>(
        attributes, tAttributes));
    REQUIRE(tAttributes.length() == attributes.size());

    std::map<std::string, std::string> roundTrip;
    REQUIRE(STI::Network::convert<STI::TNetwork::TAttributeTupleSeq, std::map<std::string, std::string>>(
        tAttributes, roundTrip));
    CHECK(roundTrip == attributes);
}

TEST_CASE("NetworkConvert: convertBuffer copies char buffers into CORBA octet sequences")
{
    const std::array<char, 6> source = {'s', 't', 'i', '\0', '3', '!'};

    STI::TNetwork::OctetSeq tBuffer;
    REQUIRE(STI::Network::convertBuffer(source.data(), static_cast<unsigned>(source.size()), tBuffer));

    REQUIRE(tBuffer.length() == source.size());
    for (CORBA::ULong i = 0; i < tBuffer.length(); ++i) {
        CHECK(tBuffer[i] == static_cast<CORBA::Octet>(source.at(i)));
    }
}

TEST_CASE("NetworkConvert: convertBuffer exposes CORBA octet sequences as char buffers")
{
    const std::array<char, 6> expected = {'n', 'e', 't', '\0', 'o', 'k'};
    STI::TNetwork::OctetSeq tBuffer;
    tBuffer.length(static_cast<CORBA::ULong>(expected.size()));
    for (CORBA::ULong i = 0; i < tBuffer.length(); ++i) {
        tBuffer[i] = static_cast<CORBA::Octet>(expected.at(i));
    }

    const char* output = nullptr;
    REQUIRE(STI::Network::convertBuffer(tBuffer, output));

    REQUIRE(output != nullptr);
    for (std::size_t i = 0; i < expected.size(); ++i) {
        CHECK(output[i] == expected.at(i));
    }
}

TEST_CASE("NetworkConvert: MixedValue scalar types round trip")
{
    {
        STI::Utils::MixedValue value(true);
        auto roundTrip = STI::Network::convert<STI::TNetwork::TMixedValue, STI::Utils::MixedValue>(
            STI::Network::convert<STI::Utils::MixedValue, STI::TNetwork::TMixedValue>(value));
        CHECK(roundTrip.getBoolean());
    }

    {
        STI::Utils::MixedValue value(42);
        auto roundTrip = STI::Network::convert<STI::TNetwork::TMixedValue, STI::Utils::MixedValue>(
            STI::Network::convert<STI::Utils::MixedValue, STI::TNetwork::TMixedValue>(value));
        CHECK(roundTrip.getInt() == 42);
    }

    {
        STI::Utils::MixedValue value(3.5);
        auto roundTrip = STI::Network::convert<STI::TNetwork::TMixedValue, STI::Utils::MixedValue>(
            STI::Network::convert<STI::Utils::MixedValue, STI::TNetwork::TMixedValue>(value));
        CHECK(roundTrip.getDouble() == Catch::Approx(3.5));
    }

    {
        STI::Utils::MixedValue value("ready");
        auto roundTrip = STI::Network::convert<STI::TNetwork::TMixedValue, STI::Utils::MixedValue>(
            STI::Network::convert<STI::Utils::MixedValue, STI::TNetwork::TMixedValue>(value));
        CHECK(roundTrip.getString() == "ready");
    }
}

TEST_CASE("NetworkConvert: MixedValue VectorInt converts to CORBA valuesInt")
{
    const std::vector<int> expected = {1, -2, 3, 5};
    STI::Utils::MixedValue value;
    value.setValue(expected);

    auto tValue = STI::Network::convert<STI::Utils::MixedValue, STI::TNetwork::TMixedValue>(value);

    REQUIRE(tValue._d() == STI::TNetwork::TMixedValueType::MixedValueVectorInt);
    REQUIRE(tValue.valuesInt().length() == expected.size());
    for (CORBA::ULong i = 0; i < tValue.valuesInt().length(); ++i) {
        CHECK(tValue.valuesInt()[i] == expected.at(i));
    }
}

TEST_CASE("NetworkConvert: CORBA MixedValue valuesInt converts to MixedValue VectorInt")
{
    const std::vector<int> expected = {8, 13, -21, 34};
    STI::TNetwork::TMixedValue tValue;
    tValue.valuesInt(STI::TNetwork::TMixedValue::_valuesInt_seq());
    tValue.valuesInt().length(static_cast<CORBA::ULong>(expected.size()));
    for (CORBA::ULong i = 0; i < tValue.valuesInt().length(); ++i) {
        tValue.valuesInt()[i] = expected.at(i);
    }

    auto value = STI::Network::convert<STI::TNetwork::TMixedValue, STI::Utils::MixedValue>(tValue);

    REQUIRE(value.getType() == STI::Utils::MixedValueType::VectorInt);
    const std::vector<int>* values = nullptr;
    REQUIRE(value.getFlatVector(values));
    REQUIRE(values != nullptr);
    CHECK(*values == expected);
}

TEST_CASE("NetworkConvert: MixedValue vectors and file IDs round trip")
{
    STI::Utils::MixedValue heterogeneous;
    heterogeneous.addValue("label");
    heterogeneous.addValue(5);
    heterogeneous.addValue(2.25);

    auto tHeterogeneous = STI::Network::convert<STI::Utils::MixedValue, STI::TNetwork::TMixedValue>(heterogeneous);
    auto heterogeneousRoundTrip =
        STI::Network::convert<STI::TNetwork::TMixedValue, STI::Utils::MixedValue>(tHeterogeneous);

    REQUIRE(heterogeneousRoundTrip.getType() == STI::Utils::MixedValueType::Vector);
    REQUIRE(heterogeneousRoundTrip.getVector().size() == 3);
    CHECK(heterogeneousRoundTrip.getVector().at(0).getString() == "label");
    CHECK(heterogeneousRoundTrip.getVector().at(1).getInt() == 5);
    CHECK(heterogeneousRoundTrip.getVector().at(2).getDouble() == Catch::Approx(2.25));

    STI::Utils::MixedValue intVector;
    intVector.setValue(std::vector<int>{1, 2, 3, 5});

    auto tIntVector = STI::Network::convert<STI::Utils::MixedValue, STI::TNetwork::TMixedValue>(intVector);
    auto intVectorRoundTrip = STI::Network::convert<STI::TNetwork::TMixedValue, STI::Utils::MixedValue>(tIntVector);

    const std::vector<int>* roundTripInts = nullptr;
    REQUIRE(intVectorRoundTrip.getFlatVector(roundTripInts));
    REQUIRE(roundTripInts != nullptr);
    CHECK(*roundTripInts == std::vector<int>{1, 2, 3, 5});

    const auto fileID = makeFileID();
    STI::Utils::MixedValue fileValue(fileID);
    auto tFileValue = STI::Network::convert<STI::Utils::MixedValue, STI::TNetwork::TMixedValue>(fileValue);
    auto fileRoundTrip = STI::Network::convert<STI::TNetwork::TMixedValue, STI::Utils::MixedValue>(tFileValue);

    REQUIRE(fileRoundTrip.getType() == STI::Utils::MixedValueType::File);
    checkFileID(fileRoundTrip.getFileID(), fileID);
}

TEST_CASE("NetworkConvert: TimeStamp and FileID round trip")
{
    const STI::Utils::TimeStamp timestamp(2026, 5, 9, 23, 59, 58, 7, 8, 9);
    auto tTimestamp = STI::Network::convert<STI::Utils::TimeStamp, STI::TNetwork::TTimeStamp>(timestamp);
    auto timestampRoundTrip = STI::Network::convert<STI::TNetwork::TTimeStamp, STI::Utils::TimeStamp>(tTimestamp);

    CHECK(timestampRoundTrip == timestamp);

    const auto fileID = makeFileID();
    auto tFileID = STI::Network::convert<STI::Utils::FileID, STI::TNetwork::TFileID>(fileID);
    auto fileIDRoundTrip = STI::Network::convert<STI::TNetwork::TFileID, STI::Utils::FileID>(tFileID);

    checkFileID(fileIDRoundTrip, fileID);
}
