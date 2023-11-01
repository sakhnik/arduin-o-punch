#ifdef BUILD_TEST

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
    CHECK(0 == data[3]);

    Punch(0xfe, 0xdeadbe).Serialize(data, 1);
    CHECK(0xfe == data[1]);
    CHECK(0xbe == data[2]);
    CHECK(0xad == data[3]);
    CHECK(0xde == data[4]);
}

TEST_CASE("Punch deserialize")
{
    const uint8_t data[] = {0xde, 0xad, 0xbe, 0xef, 0x12};
    auto p1 = Punch(data, 0);
    CHECK(0xde == p1.GetStation());
    CHECK(0xefbead == p1.GetTimestamp());

    auto p2 = Punch(data, 1);
    CHECK(0xad == p2.GetStation());
    CHECK(0x12efbe == p2.GetTimestamp());
}

#endif //BUILD_TEST
