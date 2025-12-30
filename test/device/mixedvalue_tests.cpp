#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <catch2/catch_approx.hpp>

#include <sti/utils/MixedValue.h>
#include <sti/utils/FileID.h>

#include <cmath>
#include <memory>
#include <string>
#include <vector>

using STI::Utils::FileID;
using STI::Utils::MixedValue;
using STI::Utils::MixedValueType;

TEST_CASE("MixedValue: default and primitive constructors") {
    MixedValue empty;
    CHECK(empty.isEmpty());
    CHECK(empty.getType() == MixedValueType::Empty);
    CHECK(empty.getString().empty());
    CHECK(std::isnan(empty.getNumber()));

    MixedValue boolean(true);
    CHECK(boolean.getType() == MixedValueType::Boolean);
    CHECK(boolean.getBoolean() == true);
    CHECK(boolean.isNumber());

    MixedValue integer(42);
    CHECK(integer.getType() == MixedValueType::Int);
    CHECK(integer.getInt() == 42);
    CHECK(integer.getDouble() == Catch::Approx(42.0));

    MixedValue dbl(3.5);
    CHECK(dbl.getType() == MixedValueType::Double);
    CHECK(dbl.getDouble() == Catch::Approx(3.5));
}

TEST_CASE("MixedValue: string and vector constructors") {
    MixedValue str("abc");
    CHECK(str.getType() == MixedValueType::String);
    CHECK(str.getString() == "abc");

    std::vector<std::string> strings{"one", "two"};
    MixedValue strVec(strings);
    REQUIRE(strVec.getType() == MixedValueType::Vector);
    const auto& vec = strVec.getVector();
    REQUIRE(vec.size() == 2);
    CHECK(vec[0].getString() == "one");
    CHECK(vec[1].getString() == "two");
}

TEST_CASE("MixedValue: setValue and addValue promote types") {
    MixedValue number(1);
    number.addValue(2);
    number.addValue(3);

    const std::vector<int>* ints = nullptr;
    REQUIRE(number.getType() == MixedValueType::VectorInt);
    REQUIRE(number.getFlatVector(ints));
    REQUIRE(ints != nullptr);
    REQUIRE(ints->size() == 3);
    CHECK((*ints)[0] == 1);
    CHECK((*ints)[1] == 2);
    CHECK((*ints)[2] == 3);

    number.setValue(5.25);
    CHECK(number.getType() == MixedValueType::Double);
    CHECK(number.getDouble() == Catch::Approx(5.25));
    CHECK(number.getInt() == 5);  // conversion through getNumber
}

TEST_CASE("MixedValue: heterogeneous vectors and labeled pairs") {
    MixedValue mixed;
    mixed.addValue("first", 7);
    mixed.addValue(std::string("plain"));

    REQUIRE(mixed.getType() == MixedValueType::Vector);
    const auto& vec = mixed.getVector();
    REQUIRE(vec.size() == 2);

    // labeled pair is itself a vector of two elements: label, value
    const auto& pair = vec[0].getVector();
    REQUIRE(pair.size() == 2);
    CHECK(pair[0].getString() == "first");
    CHECK(pair[1].getInt() == 7);

    CHECK(vec[1].getString() == "plain");

    std::vector<MixedValueType> expectedTypes{MixedValueType::Vector, MixedValueType::String};
    CHECK(mixed.isType(expectedTypes));
}

TEST_CASE("MixedValue: comparisons obey type and value") {
    MixedValue oneA(1);
    MixedValue oneB(1);
    MixedValue oneDouble(1.0);

    CHECK(oneA == oneB);
    CHECK(oneA != oneDouble);  // type differs
    CHECK_FALSE(oneA == MixedValue(2));

    MixedValue vecA;
    vecA.addValue(1);
    vecA.addValue(std::string("hi"));

    MixedValue vecB;
    vecB.addValue(1);
    vecB.addValue(std::string("hi"));

    CHECK(vecA == vecB);

    vecB.addValue(3);
    CHECK(vecA != vecB);
}

TEST_CASE("MixedValue: getters convert when possible") {
    MixedValue numericString(std::string("3.14"));
    CHECK(numericString.getDouble() == Catch::Approx(3.14));
    CHECK(numericString.getBoolean());  // non-zero value

    MixedValue boolValue(true);
    CHECK(boolValue.getNumber() == Catch::Approx(1.0));

    MixedValue fileValue;
    FileID fid;
    fid.filename = "data.bin";
    fid.path = "/tmp";
    fid.origin = "originA";
    fileValue.setValue(fid);

    FileID fidOut = fileValue.getFileID();
    CHECK(fidOut.filename == "data.bin");
    CHECK(fidOut.path == "/tmp");
    CHECK(fidOut.origin == "originA");
}

TEST_CASE("MixedValue: isType(Number) recurses for non-numeric values", "[mixedvalue]") {
    MixedValue stringValue(std::string("abc"));

    // Calling isType(Number) triggers isNumber(), which calls isType(Number) again.
    // On non-number types this infinite recursion overflows the stack.
    CHECK_FALSE(stringValue.isType(MixedValueType::Number));
}

TEST_CASE("MixedValue: empty comparison does not fall through to boolean logic", "[mixedvalue]") {
    MixedValue emptyA;
    MixedValue emptyB;

    // Should be equal purely because both are Empty; no boolean fallback.
    CHECK(emptyA == emptyB);
}

TEST_CASE("MixedValue: null Binary and Image values compare equal", "[mixedvalue]") {
    MixedValue nullBinaryA(std::shared_ptr<STI::Utils::BinaryData>{});
    MixedValue nullBinaryB(std::shared_ptr<STI::Utils::BinaryData>{});
    CHECK(nullBinaryA == nullBinaryB);  // currently fails because operator== requires non-null

    MixedValue nullImageA(std::shared_ptr<STI::Utils::Image>{});
    MixedValue nullImageB(std::shared_ptr<STI::Utils::Image>{});
    CHECK(nullImageA == nullImageB);  // currently fails for the same reason
}

TEST_CASE("MixedValue: getBoolean treats empty as falsy", "[mixedvalue]") {
    MixedValue emptyValue;
    CHECK(emptyValue.isEmpty());
    CHECK_FALSE(emptyValue.getBoolean());  // currently returns true via NaN != 0
}
