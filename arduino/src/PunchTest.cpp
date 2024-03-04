#ifdef BUILD_TEST

#include "doctest/doctest.h"
#include "Punch.h"

using namespace AOP;

TEST_CASE("Punch serialize")
{
    uint8_t data[8] = {};
    Punch p1(1, 3);
    p1.SerializeStation(data, 0);
    p1.SerializeTimestamp(data, 1);
    CHECK(1 == data[0]);
    CHECK(3 == data[1]);
    CHECK(0 == data[2]);
    CHECK(0 == data[3]);

    Punch p2(0xfe, 0xdeadbe);
    p2.SerializeStation(data, 1);
    p2.SerializeTimestamp(data, 2, 0xdeadbe - 0x1234);
    CHECK(0xfe == data[1]);
    CHECK(0x34 == data[2]);
    CHECK(0x12 == data[3]);
}

TEST_CASE("Punch deserialize")
{
    const uint8_t data[] = {0xde, 0xad, 0xbe, 0x7f, 0x12};
    CHECK(0xde == Punch::GetStation(data, 0));
    CHECK(0x7fbead == Punch::GetTimestamp(data, 1));

    CHECK(0xad == Punch::GetStation(data, 1));
    CHECK(0x7fbe + 12345 == Punch::GetTimestamp(data, 2, 12345));
}

TEST_CASE("Punch decreasing timestamp")
{
    uint8_t data[8] = {};
    Punch p1(31, 12345);
    p1.SerializeTimestamp(data, 0);
    CHECK(12345 == Punch::GetTimestamp(data, 0));
    p1.SerializeTimestamp(data, 0, 12340);
    CHECK(12345 == Punch::GetTimestamp(data, 0, 12340));
    p1.SerializeTimestamp(data, 0, 12350);
    CHECK(12345 == Punch::GetTimestamp(data, 0, 12350));
}

#endif //BUILD_TEST
