#ifdef BUILD_TEST

#include "../src/PunchCard.h"
#include <emscripten/bind.h>
#include <emscripten/em_js.h>

using namespace emscripten;

EM_JS(void, update_cell, (int block, int offset, uint8_t val, bool changed), {
  console.log('I received: ' + [block, offset, val, changed]);
});

class WebMifare : public AOP::IMifare
{
    uint8_t BlockToSector(uint8_t block) const override
    {
        return block / 4;
    }

    uint8_t AuthenticateSectorWithKeyA(uint8_t sector, const uint8_t *key) override
    {
        return 1;
    }

    uint8_t ReadBlock(uint8_t block, uint8_t *data, uint8_t &dataSize) override
    {
        return 1;
    }

    uint8_t WriteBlock(uint8_t block, const uint8_t *data, uint8_t dataSize) override
    {
        return 1;
    }
};

EMSCRIPTEN_BINDINGS(my_module) {
    class_<WebMifare>("WebMifare")
        .constructor<>();

    class_<AOP::Punch>("Punch")
        .constructor<uint8_t, uint32_t>();

    class_<AOP::PunchCard>("PunchCard")
        .constructor<WebMifare *, const uint8_t *>()
        .function("Punch", &AOP::PunchCard::Punch)
        .function("Clear", &AOP::PunchCard::Clear);
}

#endif //BUILD_TEST
