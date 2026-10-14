
#ifdef BUILD_TUI

#include "PunchCard.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <ctime>
#include <format>
#include <iostream>
#include <sstream>
#include <string>

namespace {

using Mifare = AOP::IMifare;
constexpr size_t BlockSize = Mifare::BLOCK_SIZE;
constexpr size_t BlockCount = Mifare::SECTOR_COUNT * Mifare::BLOCKS_PER_SECTOR;

using Memory = std::array<uint8_t, BlockCount * BlockSize>;

constexpr const char* Reset = "\033[0m";
constexpr const char* Changed = "\033[30;103m";

std::string ToHex(const uint8_t *data, size_t size)
{
    std::string result;
    for (size_t i = 0; i < size; ++i)
        result += std::format("{:02X}", unsigned(data[i]));
    return result;
}

bool ParseHex(const std::string &text, uint8_t *data, size_t size)
{
    if (text.size() != size * 2)
        return false;

    for (size_t i = 0; i < size; ++i) {
        auto hex = text.substr(i * 2, 2);
        size_t used = 0;
        unsigned long value;

        try {
            value = std::stoul(hex, &used, 16);
        } catch (...) {
            return false;
        }

        if (used != 2 || value > 255)
            return false;

        data[i] = static_cast<uint8_t>(value);
    }

    return true;
}

bool ParseUnsigned(const std::string &text, unsigned &value)
{
    if (text.empty())
        return false;

    size_t used = 0;

    try {
        auto parsed = std::stoul(text, &used, 10);
        if (used != text.size() || parsed > UINT32_MAX)
            return false;
        value = static_cast<unsigned>(parsed);
        return true;
    } catch (...) {
        return false;
    }
}

std::string TimeString(uint32_t seconds)
{
    return std::format("{:02}:{:02}:{:02}", seconds / 3600, seconds / 60 % 60, seconds % 60);
}

class VirtualMifare final
    : public Mifare
{
public:
    VirtualMifare()
    {
        memory.fill(0xFF);

        // A blank card uses the default key in every sector.
        for (unsigned sector = 0; sector < SECTOR_COUNT; ++sector) {
            auto *trailer = Block(sector * BLOCKS_PER_SECTOR + BLOCKS_PER_SECTOR - 1);

            std::copy(KEY_DEFAULT.begin(), KEY_DEFAULT.end(), trailer);
        }
    }

    uint8_t BlockToSector(uint8_t block) const override
    {
        return block / BLOCKS_PER_SECTOR;
    }

    uint8_t AuthenticateSectorWithKeyA(uint8_t sector, const uint8_t *key) override
    {
        if (sector >= SECTOR_COUNT)
            return 1;

        const auto* trailer = Block(sector * BLOCKS_PER_SECTOR + BLOCKS_PER_SECTOR - 1);

        return std::equal(key, key + KEY_SIZE, trailer) ? 0 : 1;
    }

    uint8_t ReadBlock(uint8_t block, uint8_t *data, uint8_t &size) override
    {
        if (block >= BlockCount || size < BLOCK_SIZE)
            return 1;

        std::copy_n(Block(block), BLOCK_SIZE, data);
        size = BLOCK_SIZE;
        return 0;
    }

    uint8_t WriteBlock(uint8_t block, const uint8_t *data, uint8_t size) override
    {
        if (block >= BlockCount || size != BLOCK_SIZE)
            return 1;

        const auto offset = block * BLOCK_SIZE;
        std::copy_n(data, BLOCK_SIZE, Block(block));

        for (unsigned i = 0; i < BLOCK_SIZE; ++i)
            changed[offset + i] = memory[offset + i] != before[offset + i];

        return 0;
    }

    const uint8_t* Block(unsigned block) const
    {
        return memory.data() + block * BLOCK_SIZE;
    }

    uint8_t* Block(unsigned block)
    {
        return memory.data() + block * BLOCK_SIZE;
    }

    void BeginTracking()
    {
        before = memory;
        changed.fill(false);
    }

    bool Changed(unsigned block, unsigned byte) const
    {
        return changed[block * BLOCK_SIZE + byte];
    }

    Memory memory;

private:
    Memory before{};
    std::array<bool, BlockCount * BlockSize> changed{};
};

void ShowChanges(const VirtualMifare &mifare)
{
    unsigned changedBlocks = 0;
    unsigned changedBytes = 0;

    for (unsigned block = 0; block < BlockCount; ++block) {
        bool changed = false;

        for (unsigned i = 0; i < BlockSize; ++i) {
            if (mifare.Changed(block, i)) {
                changed = true;
                ++changedBytes;
            }
        }

        if (!changed)
            continue;

        ++changedBlocks;
        std::cout << std::format("\n[{:02}] ", block);

        const auto* data = mifare.Block(block);

        for (unsigned i = 0; i < BlockSize; ++i) {
            const bool byteChanged = mifare.Changed(block, i);
            std::cout << std::format("{}{:02X}{} ", byteChanged ? Changed : "", unsigned(data[i]), byteChanged ? Reset : "");
        }
    }

    if (!changedBlocks) {
        std::cout << "\nMemory unchanged.\n";
    } else {
        std::cout << std::format("\n\n{} byte(s) changed in {} block(s).\n", changedBytes, changedBlocks);
    }
}

void DumpBlock(const VirtualMifare &mifare, unsigned block)
{
    if (block >= BlockCount) {
        std::cout << std::format("Invalid block. Range: 0-{}\n", BlockCount - 1);
        return;
    }

    const auto* data = mifare.Block(block);
    std::cout << std::format("[{:02}] ", block);

    for (unsigned i = 0; i < BlockSize; ++i) {
        const bool changed = mifare.Changed(block, i);
        std::cout << std::format("{}{:02X}{} ", changed ? Changed : "", unsigned(data[i]), changed ? Reset : "");
    }

    std::cout << '\n';
}

void Help()
{
    std::cout
        << "Commands:\n"
        << "  key HEX               Set the card's Key A (12 hex digits)\n"
        << "  format ID [SECTOR]    Format the card; omit sector to randomize\n"
        << "  punch STATION [TIME]  Punch; TIME is seconds since midnight\n"
        << "  clear                 Clear punch history\n"
        << "  readout               Decode and display punches\n"
        << "  dump [BLOCK]          Display a block or all blocks\n"
        << "  help                  Show this help\n"
        << "  quit                  Exit\n\n"
        << "Changed bytes are highlighted. Only changed blocks are shown\n"
        << "after each operation. Dump preserves the last change highlighting.\n";
}

} // namespace

int main()
{
    VirtualMifare mifare;
    Mifare::KeyT key = Mifare::KEY_DEFAULT;

    Help();

    std::string line;

    for (;;) {
        std::cout << "\naop> " << std::flush;

        if (!std::getline(std::cin, line))
            break;

        std::istringstream input(line);
        std::string command;

        if (!(input >> command))
            continue;

        if (command == "quit" || command == "q")
            break;

        if (command == "help" || command == "h") {
            Help();
            continue;
        }

        if (command == "dump") {
            std::string arg;
            unsigned block;

            if (!(input >> arg)) {
                for (unsigned block = 0; block < BlockCount; ++block)
                    DumpBlock(mifare, block);
                continue;
            }

            if (!ParseUnsigned(arg, block)) {
                std::cout << "Usage: dump [BLOCK]\n";
                continue;
            }

            DumpBlock(mifare, block);
            continue;
        }

        if (command == "key") {
            std::string hex;

            if (!(input >> hex) || !ParseHex(hex, key.data(), key.size())) {
                std::cout << "Expected 12 hexadecimal digits.\n";
                continue;
            }

            std::cout << "Key updated.\n";
            continue;
        }

        uint8_t result = 0;
        bool executed = true;
        mifare.BeginTracking();

        if (command == "format") {
            std::string idText, sectorText;
            unsigned id, sector = 255;

            if (!(input >> idText) || !ParseUnsigned(idText, id) || id > UINT16_MAX) {
                std::cout << "Usage: format ID [SECTOR]\n";
                continue;
            }

            if (input >> sectorText) {
                if (!ParseUnsigned(sectorText, sector) || sector >= Mifare::SECTOR_COUNT) {
                    std::cout << "Invalid sector.\n";
                    continue;
                }
            }

            AOP::PunchCard card{&mifare, key};
            result = card.Format(static_cast<uint16_t>(id), {Mifare::KEY_DEFAULT});
        } else if (command == "punch") {
            std::string stationText, timeText;
            unsigned station, timestamp;

            if (!(input >> stationText) || !ParseUnsigned(stationText, station) || station > 255) {
                std::cout << "Usage: punch STATION [TIME]\n";
                continue;
            }

            if (input >> timeText) {
                if (!ParseUnsigned(timeText, timestamp)) {
                    std::cout << "Invalid timestamp.\n";
                    continue;
                }
            } else {
                const auto now = std::time(nullptr);
                const auto local = *std::localtime(&now);
                timestamp = local.tm_hour * 3600 + local.tm_min * 60 + local.tm_sec;
            }

            if (timestamp >= 86400) {
                std::cout << "TIME must be less than 86400.\n";
                continue;
            }

            AOP::PunchCard card{&mifare, key};
            result = card.Punch(AOP::Punch{static_cast<uint8_t>(station), timestamp});
        } else if (command == "clear") {
            AOP::PunchCard card{&mifare, key};
            result = card.Format({}, {Mifare::KEY_DEFAULT});
        } else if (command == "readout") {
            AOP::PunchCard card{&mifare, key};
            AOP::PunchCard::CardReadOut readOut;
            result = card.ReadOut(readOut);

            if (result == 0) {
                std::cout << std::format("Card ID: {}\n#\tStation\t\tTime\n", readOut.cardId);

                for (size_t i = 0; i < readOut.punches.size(); ++i) {
                    const auto& punch = readOut.punches[i];
                    std::cout << std::format("{}\t{}\t\t{}\n", i + 1, unsigned(punch.GetStation()), TimeString(punch.GetTimestamp()));
                }
            }
        } else {
            executed = false;
            std::cout << "Unknown command. Type 'help'.\n";
        }

        if (!executed)
            continue;

        if (result != 0) {
            std::cout << std::format("Operation failed: error {}\n", unsigned(result));
        } else {
            std::cout << "OK\n";
        }

        ShowChanges(mifare);
    }

    return 0;
}

#endif //BUILD_TUI
