#include "PunchCard.h"

#ifdef BUILD_TEST
# include <cassert>
# include <cstring>
#else
# include <Arduino.h>
#endif

namespace AOP {

PunchCard::PunchCard(IMifare *mifare, const uint8_t *key, uint8_t *key_receiver)
    : _mifare{mifare}
    , _key{key}
    , _key_receiver{key_receiver}
{
}

uint8_t PunchCard::Punch(AOP::Punch punch, ProgressT progress)
{
    uint8_t stages = 8;
    progress(0, stages);

    // 1. read the index
    auto headerSector = _mifare->BlockToSector(HEADER_BLOCK);
    if (auto res = _Authenticate(headerSector))
        return res;
    progress(1, stages);
    uint8_t header[IMifare::BLOCK_SIZE + 2];  // need at least 18 bytes +crc
    uint8_t headerSize = sizeof(header);
    if (auto res = _mifare->ReadBlock(HEADER_BLOCK, header, headerSize))
        return res;
    progress(2, stages);
    if (header[DESC_OFFSET] == DESC_SERVICE)
    {
        int id = static_cast<int>(header[ID_OFFSET]) | (static_cast<int>(header[ID_OFFSET + 1]) << 8);
        // Service card found, remember the key
        if (!id && _key_receiver)
        {
            memcpy(_key_receiver, header + KEY_OFFSET, 6);
        }
    }
    uint8_t index = header[INDEX_OFFSET];
    auto addr = _GetPunchAddr(index - 1);
    if (auto res = _Authenticate(_mifare->BlockToSector(addr.block)))
        return res;
    progress(3, stages);
    uint8_t punchBlock[IMifare::BLOCK_SIZE + 2];
    uint8_t punchBlockSize = sizeof(punchBlock);
    if (auto res = _mifare->ReadBlock(addr.block, punchBlock, punchBlockSize))
        return res;
    progress(4, stages);

    // 2. read the last record
    AOP::Punch prevPunch{punchBlock, addr.offset};
    //   a. check the station is different
    if (prevPunch.GetStation() == punch.GetStation())
    {
        progress(stages, stages);
        return 0;
    }

    // 3. write the next record
    auto newAddr = _GetPunchAddr(index);
    if (newAddr.block == addr.block)
    {
        punch.Serialize(punchBlock, newAddr.offset);
        if (auto res = _mifare->WriteBlock(newAddr.block, punchBlock, IMifare::BLOCK_SIZE))
            return res;
    }
    else
    {
        if (newAddr.block >= IMifare::BLOCK_COUNT)
            return 1;  // The card is full
#ifdef BUILD_TEST
        assert(newAddr.offset == 0);
#endif // BUILD_TEST
        if (auto res = _Authenticate(_mifare->BlockToSector(newAddr.block)))
            return res;
        progress(5, stages);
        punchBlockSize = sizeof(punchBlock);
        if (auto res = _mifare->ReadBlock(newAddr.block, punchBlock, punchBlockSize))
            return res;
        progress(6, stages);
        punch.Serialize(punchBlock, newAddr.offset);
        if (auto res = _mifare->WriteBlock(newAddr.block, punchBlock, IMifare::BLOCK_SIZE))
            return res;
    }
    progress(7, stages);

    // 4. update the index
    header[INDEX_OFFSET] = index + 1;
    if (auto res = _Authenticate(headerSector))
        return res;
    if (auto res = _mifare->WriteBlock(HEADER_BLOCK, header, IMifare::BLOCK_SIZE))
        return res;
    progress(stages, stages);
    return 0;
}

#ifdef BUILD_TEST

uint8_t PunchCard::ReadOut(std::vector<AOP::Punch> &punches, ProgressT progress)
{
    progress(0, 1);
    // Read everything
    // Reconstruct timestamps
    auto headerSector = _mifare->BlockToSector(HEADER_BLOCK);
    if (uint8_t res = _Authenticate(headerSector))
        return res;
    uint8_t data[IMifare::BLOCK_SIZE + 2];
    uint8_t dataSize = sizeof(data);
    if (uint8_t res = _mifare->ReadBlock(HEADER_BLOCK, data, dataSize))
        return res;
    uint8_t count = data[INDEX_OFFSET];

    uint8_t prevBlock = -1;

    punches.clear();
    punches.reserve(count - 1);
    for (uint8_t index = 1; index < count; ++index)
    {
        progress(index, count);
        auto addr = _GetPunchAddr(index);
        uint8_t block = addr.block;
        uint8_t offset = addr.offset;
        if (block != prevBlock)
        {
            prevBlock = block;
            if (uint8_t res = _Authenticate(_mifare->BlockToSector(block)))
                return res;
            dataSize = sizeof(data);
            if (uint8_t res = _mifare->ReadBlock(block, data, dataSize))
                return res;
        }
        punches.push_back(AOP::Punch(data, offset));
    }
    progress(count, count);
    return 0;
}

#endif //BUILD_TEST

uint8_t PunchCard::_Authenticate(uint8_t sector)
{
    if (sector != _auth_sector)
    {
        if (auto res = _mifare->AuthenticateSectorWithKeyA(sector, _key))
            return res;
        _auth_sector = sector;
    }
    return 0;
}

PunchCard::_Address PunchCard::_GetPunchAddr(uint8_t index)
{
    int byteAddr = static_cast<int>(index) * AOP::Punch::STORAGE_SIZE;
    uint8_t offset = byteAddr % 16;
    int blockNumber = byteAddr / 16;

    // Map onto free blocks skipping through the trailer blocks
    // 0 -> 2    1 -> 4, 2 -> 5, 3 -> 6   4 -> 8 ...
    uint8_t block = (blockNumber + 2) / 3 * 4 + (blockNumber + 2) % 3;
    return {block, offset};
}

}  // namespace AOP;
