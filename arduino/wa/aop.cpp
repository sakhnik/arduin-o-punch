#ifdef BUILD_WA

#include "../src/PunchCard.h"
#include <emscripten/bind.h>
#include <emscripten/em_js.h>

using namespace emscripten;

// *INDENT-OFF*
EM_JS(void, update_cell, (int block, int offset, int val, bool changed), {
    let cell = document.getElementById(`cell_${block}_${offset}`);
    cell.textContent = val.toString(16).padStart(2, '0').toUpperCase();
    if (changed)
    {
        cell.classList.add('valueChanged');
    } else
    {
        cell.classList.remove('valueChanged');
    }
});
// *INDENT-ON*

class WebMifare : public AOP::IMifare
{
    using BlockT = std::array<uint8_t, IMifare::BLOCK_SIZE>;
    std::array<BlockT, IMifare::BLOCK_COUNT> blocks;
    uint8_t authSector = -1;
    std::vector<std::pair<uint8_t, uint8_t>> highlights;

public:
    WebMifare()
    {
        for (auto &block : blocks)
            block.fill(0xff);
        for (int sector = 0; sector < 16; ++sector) {
            std::copy(std::begin(AOP::IMifare::ACCESS_BITS), std::end(AOP::IMifare::ACCESS_BITS),
                      blocks[sector * 4 + 3].data() + 6);
        }

        for (int block = 1; block < AOP::IMifare::BLOCK_COUNT; ++block) {
            for (int offset = 0; offset < AOP::IMifare::BLOCK_SIZE; ++offset) {
                update_cell(block, offset, blocks[block][offset], false);
            }
        }
    }

    ~WebMifare()
    {
        ResetHighlights();
    }

    uint8_t BlockToSector(uint8_t block) const override
    {
        return block / 4;
    }

    uint8_t AuthenticateSectorWithKeyA(uint8_t sector, const uint8_t *key) override
    {
        authSector = sector;
        return std::equal(key, key + AOP::IMifare::KEY_SIZE, blocks[4 * sector + 3].data()) ? 0 : 1;
    }

    uint8_t ReadBlock(uint8_t block, uint8_t *data, uint8_t &) override
    {
        if (BlockToSector(block) != authSector)
            return 1;
        const uint8_t *src = blocks[block].data();
        std::copy(src, src + AOP::IMifare::BLOCK_SIZE, data);
        return 0;
    }

    uint8_t WriteBlock(uint8_t block, const uint8_t *data, uint8_t blockSize) override
    {
        if (BlockToSector(block) != authSector)
            return 1;
        uint8_t *dst = blocks[block].data();
        for (int offset = 0; offset < AOP::IMifare::BLOCK_SIZE; ++offset) {
            auto val = data[offset];
            if (dst[offset] != val) {
                dst[offset] = val;
                update_cell(block, offset, val, true);
                highlights.emplace_back(block, offset);
            } else {
                update_cell(block, offset, val, false);
            }
        }
        return 0;
    }

    void ResetHighlights()
    {
        for (auto blockOffset : highlights) {
            auto block = blockOffset.first;
            auto offset = blockOffset.second;
            update_cell(block, offset, blocks[block][offset], false);
        }
        highlights.clear();
    }
};

struct KeyHolder
{
    std::string _keyA;

    KeyHolder(const std::string &key)
        : _keyA{key}
    {
    }

    const uint8_t *GetKeyA()
    {
        return reinterpret_cast<const uint8_t *>(_keyA.data());
    }
};

class PunchCard
    : protected KeyHolder
    , AOP::PunchCard
{
    WebMifare *_mifare;

public:
    PunchCard(WebMifare *mifare, std::string key)
        : KeyHolder{key}
        , AOP::PunchCard{mifare, GetKeyA()}
        , _mifare{mifare}
    {
    }

    uint8_t Format(uint16_t cardId, uint8_t startSector)
    {
        _mifare->ResetHighlights();
        return AOP::PunchCard::Format(cardId, startSector);
    }

    uint8_t Punch(uint8_t stationId, uint32_t timestamp)
    {
        _mifare->ResetHighlights();
        return AOP::PunchCard::Punch(AOP::Punch{stationId, timestamp});
    }

    struct PunchData
    {
        uint8_t stationId;
        uint32_t timestamp;

        PunchData(uint8_t stationId = 0, uint32_t timestamp = 0)
            : stationId{stationId}
            , timestamp{timestamp}
        {
        }
    };

    std::vector<PunchData> lastReadout;

    const std::vector<PunchData>& GetLastReadout()
    {
        return lastReadout;
    }

    uint8_t ReadOut()
    {
        AOP::PunchCard::PunchesT punches;
        if (auto res = AOP::PunchCard::ReadOut(punches)) {
            return res;
        }
        lastReadout.clear();
        for (const auto &p : punches) {
            lastReadout.emplace_back(p.GetStation(), p.GetTimestamp());
        }
        return 0;
    }
};

EMSCRIPTEN_BINDINGS(AOP)
{
    class_<WebMifare>("WebMifare")
    .constructor<>();

    value_object<PunchCard::PunchData>("Punch")
    .field("stationId", &PunchCard::PunchData::stationId)
    .field("timestamp", &PunchCard::PunchData::timestamp);

    register_vector<PunchCard::PunchData>("VectorPunch");

    class_<PunchCard>("PunchCard")
    .constructor<WebMifare *, std::string>()
    .function("Format", &PunchCard::Format)
    .function("Punch", &PunchCard::Punch)
    .function("ReadOut", &PunchCard::ReadOut)
    .function("GetLastReadout", &PunchCard::GetLastReadout);
    //.function("Clear", &PunchCard::Clear);
}

#endif //BUILD_WA
