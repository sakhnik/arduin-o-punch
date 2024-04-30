#ifdef BUILD_TEST

#include <doctest/doctest.h>
#include "RingBuffer.h"
#include <iterator>
#include <numeric>

using namespace AOP;

TEST_CASE("RingBuf looping")
{
    RingBuffer<13> buf;
    CHECK(buf.Size() == 0);
    uint8_t TEST_DATA[20];
    std::iota(std::begin(TEST_DATA), std::end(TEST_DATA), 0);
    for (int i = 0; i < 100; ++i) {
        CAPTURE(i);
        buf.Add(TEST_DATA, 3);
        REQUIRE(buf.Size() == 3);
        buf.Add(TEST_DATA + 3, 4);
        REQUIRE(buf.Size() == 7);
        buf.Add(TEST_DATA + 7, 5);
        REQUIRE(buf.Size() == 12);

        int off = 0;
        while (buf.Size()) {
            auto chunk = buf.Get(31);
            if (!chunk.size)
                break;
            REQUIRE(std::equal(chunk.data, chunk.data + chunk.size, TEST_DATA + off));
            off += chunk.size;
        }
    }
}

TEST_CASE("RingBuf too large")
{
    RingBuffer<5> buf;
    buf.Add(reinterpret_cast<const uint8_t *>("hello, world"), 12);
    REQUIRE(buf.Size() == 5);
    auto chunk = buf.Get(31);
    REQUIRE(chunk.size == 5);
    REQUIRE(0 == memcmp("world", chunk.data, chunk.size));

    buf.Add(reinterpret_cast<const uint8_t *>("asdf"), 2);
    buf.Add(reinterpret_cast<const uint8_t *>("hello, world"), 12);
    REQUIRE(buf.Size() == 5);
    chunk = buf.Get(31);
    REQUIRE(chunk.size == 3);
    REQUIRE(0 == memcmp("wor", chunk.data, chunk.size));
    REQUIRE(buf.Size() == 2);
    chunk = buf.Get(31);
    REQUIRE(chunk.size == 2);
    REQUIRE(0 == memcmp("ld", chunk.data, chunk.size));
    REQUIRE(buf.Size() == 0);
}

#endif //BUILD_TEST
