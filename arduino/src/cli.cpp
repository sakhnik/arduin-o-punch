#ifdef BUILD_CLI

#include "PunchCard.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <array>

namespace {

std::string ToHex(const uint8_t *bytes, size_t length)
{
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (size_t i = 0; i < length; ++i) {
        ss << std::setw(2) << static_cast<int>(bytes[i]);
    }
    return ss.str();
}

void FromHex(const std::string &hexString, uint8_t *data)
{
    for (size_t i = 0; i < hexString.length(); i += 2) {
        *data++ = std::stoul(hexString.substr(i, 2), nullptr, 16);
    }
}

} //namespace;

struct CliMifare : AOP::IMifare
{
    uint8_t BlockToSector(uint8_t block) const override
    {
        return block / 4;
    }

    uint8_t AuthenticateSectorWithKeyA(uint8_t sector, const uint8_t *key) override
    {
        std::cout << "A " << static_cast<unsigned>(sector) << " " << ToHex(key, AOP::IMifare::KEY_SIZE) << std::endl;
        std::string response;
        getline(std::cin, response);
        return response == "OK" ? 0 : 1;
    }

    uint8_t ReadBlock(uint8_t block, uint8_t *data, uint8_t &) override
    {
        std::cout << "R " << static_cast<unsigned>(block) << std::endl;
        std::string response;
        getline(std::cin, response);
        std::istringstream iss{response};
        iss >> response;
        if (response != "OK") {
            return 1;
        }
        iss >> response;
        FromHex(response, data);
        return 0;
    }

    uint8_t WriteBlock(uint8_t block, const uint8_t *data, uint8_t blockSize) override
    {
        std::cout << "W " << static_cast<unsigned>(block) << " " << ToHex(data, blockSize) << std::endl;
        std::string response;
        getline(std::cin, response);
        return response == "OK" ? 0 : 1;
    }
};

int main()
{
    std::array<uint8_t, AOP::IMifare::KEY_SIZE> key;

    std::string line;
    while (getline(std::cin, line)) {
        std::string cmd;
        std::istringstream iss{line};
        iss >> cmd;
        if (cmd == "KEY") {
            iss >> cmd;
            FromHex(cmd, key.data());
            std::cout << "OK" << std::endl;
            continue;
        }
        if (cmd == "PUNCH") {
            unsigned stationId{};
            uint32_t timestamp;
            iss >> stationId >> timestamp;
            CliMifare mifare;
            AOP::PunchCard punchCard{&mifare, key.data()};
            if (auto res = punchCard.Punch(AOP::Punch{static_cast<uint8_t>(stationId), timestamp})) {
                std::cout << "ERR " << static_cast<unsigned>(res) << std::endl;
            } else {
                std::cout << "OK" << std::endl;
            }
            continue;
        }
        if (cmd == "CLEAR") {
            CliMifare mifare;
            AOP::PunchCard punchCard{&mifare, key.data()};
            if (auto res = punchCard.Clear()) {
                std::cout << "ERR " << static_cast<unsigned>(res) << std::endl;
            } else {
                std::cout << "OK" << std::endl;
            }
            continue;
        }
        if (cmd == "QUIT") {
            std::cout << "OK" << std::endl;
            break;
        }
    }
}

#endif //BUILD_CLI
