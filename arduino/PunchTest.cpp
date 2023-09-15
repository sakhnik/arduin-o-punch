#include "doctest/doctest.h"
#include "Punch.h"

using namespace AOP;

TEST_CASE("Punch serialize")
{
    uint8_t data[Punch::STORAGE_SIZE * 2] = {};
    Punch(1, 3).Serialize(data, 0);
    CHECK(1 == data[0]);
    CHECK(3 == data[1]);
    CHECK(0 == data[2]);

    Punch(0xfe, 0xdead).Serialize(data, 1);
    CHECK(0xfe == data[1]);
    CHECK(0xad == data[2]);
    CHECK(0xde == data[3]);
}

TEST_CASE("Punch deserialize")
{
    const uint8_t data[] = {0xde, 0xad, 0xbe, 0xef};
    auto p1 = Punch(data, 0);
    CHECK(0xde == p1.GetStation());
    CHECK(0xbead == p1.GetTimestamp());

    auto p2 = Punch(data, 1);
    CHECK(0xad == p2.GetStation());
    CHECK(0xefbe == p2.GetTimestamp());
}
