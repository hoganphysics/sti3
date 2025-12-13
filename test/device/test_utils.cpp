#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <sti/utils/utils.h>

#include <string>
#include <vector>

TEST_CASE("Utils: stringToValue and valueToString") {
    std::string intString = "42";
    int intValue = 0;
    bool success = STI::Utils::stringToValue(intString, intValue);
    REQUIRE(success);
    REQUIRE(intValue == 42);

    std::string floatString = "3.14159";
    double floatValue = 0.0;
    success = STI::Utils::stringToValue(floatString, floatValue);
    REQUIRE(success);
    // REQUIRE(floatValue == Catch::Approx(3.14159).epsilon(0.00001));
    REQUIRE_THAT(floatValue, Catch::Matchers::WithinRel(3.14159, 0.00001));

    std::string convertedIntString = STI::Utils::valueToString(intValue);
    REQUIRE(convertedIntString == "42");

    std::string convertedFloatString = STI::Utils::valueToString(floatValue, "", std::ios::dec, 5);
    REQUIRE(convertedFloatString == "3.1416"); // Rounded to 5 significant digits

    REQUIRE(STI::Utils::stringToValue("invalid", intValue) == false);
}

TEST_CASE("Utils: trim") {
    std::string untrimmed = "   Hello, World! \n";
    std::string trimmed = STI::Utils::trim(untrimmed);
    REQUIRE(trimmed == "Hello, World!");
    std::string allWhitespace = "    \n\t  ";
    std::string trimmedWhitespace = STI::Utils::trim(allWhitespace);
    REQUIRE(trimmedWhitespace == "");

    std::string noTrimNeeded = "NoTrim";;
    std::string trimmedNoTrim = STI::Utils::trim(noTrimNeeded);
    REQUIRE(trimmedNoTrim == "NoTrim");

    std::string customTrim = "***Custom***";
    std::string trimmedCustom = STI::Utils::trim(customTrim, "*");
    REQUIRE(trimmedCustom == "Custom");
}

TEST_CASE("Utils: replaceChar and replaceChars") {
    std::string original = "banana";
    std::string replacedChar = STI::Utils::replaceChar(original, "a", "o");
    REQUIRE(replacedChar == "bonono");

    std::string replacedChars = STI::Utils::replaceChars(original, "an", "X");
    REQUIRE(replacedChars == "bXXXXX");
}

TEST_CASE("Utils: splitString") {
    std::string toSplit = "one,two,,three,";
    std::vector<std::string> result;
    STI::Utils::splitString(toSplit, ",", result);
    REQUIRE(result.size() == 5);
    REQUIRE(result[0] == "one");
    REQUIRE(result[1] == "two");
    REQUIRE(result[2] == "");
    REQUIRE(result[3] == "three");
    REQUIRE(result[4] == "");

    std::string noDelimiter = "nosplit";
    STI::Utils::splitString(noDelimiter, ",", result);
    REQUIRE(result.size() == 1);
    REQUIRE(result[0] == "nosplit");

    std::string emptyDelimiter = "test";
    STI::Utils::splitString(emptyDelimiter, "", result);
    REQUIRE(result.size() == 1);
    REQUIRE(result[0] == "test");

    std::string toSplit2 = "one|two|three";
    STI::Utils::splitString(toSplit2, "|", result);
    REQUIRE(result.size() == 3);
    REQUIRE(result[0] == "one");
    REQUIRE(result[1] == "two");
    REQUIRE(result[2] == "three");
}

TEST_CASE("Utils: printTimeFormated") {
    double time_ns = 23531033; // 23ms, 531us, 33ns
    std::string formatted = STI::Utils::printTimeFormated(time_ns);
    REQUIRE(formatted == "23ms|531us|33ns");

    time_ns = 53010033; // 0s, 53ms, 10us, 33ns
    formatted = STI::Utils::printTimeFormated(time_ns);
    REQUIRE(formatted == "53ms|10us|33ns");

    time_ns = 101033; // 0s, 0ms, 101us, 33ns
    formatted = STI::Utils::printTimeFormated(time_ns);
    REQUIRE(formatted == "101us|33ns");

    time_ns = 33; // 0s, 0ms, 0us, 33ns
    formatted = STI::Utils::printTimeFormated(time_ns);
    REQUIRE(formatted == "33ns");
}

