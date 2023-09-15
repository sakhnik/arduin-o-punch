#include "PunchCard.h"

namespace AOP {

PunchCard::PunchCard(IMifare *mifare, const uint8_t *key)
    : _mifare{mifare}
    , _key{key}
{
}

int PunchCard::Punch(AOP::Punch punch, void(*progress)(int, int))
{
    int stages = 8;
    progress(0, stages);

    // 1. read the index
    auto headerSector = _mifare->BlockToSector(HEADER_BLOCK);
    if (int res = _Authenticate(headerSector))
        return res;
    progress(1, stages);
    uint8_t header[IMifare::BLOCK_SIZE];
    if (int res = _mifare->ReadBlock(HEADER_BLOCK, header))
        return res;
    progress(2, stages);
    uint8_t index = header[5];
    auto blockOffset = _GetPunchAddr(index - 1);
    if (int res = _Authenticate(_mifare->BlockToSector(blockOffset.first)))
        return res;
    progress(3, stages);
    uint8_t punchBlock[IMifare::BLOCK_SIZE];
    if (int res = _mifare->ReadBlock(blockOffset.first, punchBlock))
        return res;
    progress(4, stages);

    // 2. read the last record
    AOP::Punch prevPunch{punchBlock, blockOffset.second};
    //   a. check the station is different
    if (prevPunch.GetStation() == punch.GetStation())
    {
        progress(stages, stages);
        return 0;
    }

    // 3. write the next record
    auto newBlockOffset = _GetPunchAddr(index);
    if (newBlockOffset.first == blockOffset.first)
    {
        punch.Serialize(punchBlock, newBlockOffset.second);
        _mifare->WriteBlock(newBlockOffset.first, punchBlock);
    }
    else
    {
        if (newBlockOffset.first >= IMifare::BLOCK_COUNT)
            return -1;  // The card is full
        //assert(newBlockOffset.second == 0);
        if (int res = _Authenticate(_mifare->BlockToSector(newBlockOffset.first)))
            return res;
        progress(5, stages);
        if (int res = _mifare->ReadBlock(newBlockOffset.first, punchBlock))
            return res;
        progress(6, stages);
        punch.Serialize(punchBlock, newBlockOffset.second);
        if (int res = _mifare->WriteBlock(newBlockOffset.first, punchBlock))
            return res;
    }
    progress(7, stages);

    // 4. update the index
    header[5] = index + 1;
    if (int res = _Authenticate(headerSector))
        return res;
    if (int res = _mifare->WriteBlock(HEADER_BLOCK, header))
        return res;
    progress(stages, stages);
    return 0;
}

int PunchCard::ReadOut(std::vector<AOP::Punch> &punches, void (*progress)(int, int))
{
    progress(0, 1);
    // Read everything
    // Reconstruct timestamps
    auto headerSector = _mifare->BlockToSector(HEADER_BLOCK);
    if (int res = _Authenticate(headerSector))
        return res;
    uint8_t data[IMifare::BLOCK_SIZE];
    if (int res = _mifare->ReadBlock(HEADER_BLOCK, data))
        return res;
    uint8_t count = data[5];

    uint8_t prevBlock = -1;

    punches.clear();
    punches.reserve(count - 1);
    for (uint8_t index = 1; index < count; ++index)
    {
        progress(index, count);
        auto blockOffset = _GetPunchAddr(index);
        uint8_t block = blockOffset.first;
        uint8_t offset = blockOffset.second;
        if (block != prevBlock)
        {
            prevBlock = block;
            if (int res = _Authenticate(_mifare->BlockToSector(block)))
                return res;
            if (int res = _mifare->ReadBlock(block, data))
                return res;
        }
        punches.push_back(AOP::Punch(data, offset));
    }
    progress(count, count);
    return 0;
}

int PunchCard::_Authenticate(uint8_t sector)
{
    if (sector != _auth_sector)
    {
        if (int res = _mifare->AuthenticateSectorWithKeyA(sector, _key))
        {
            return res;
        }
        _auth_sector = sector;
    }
    return 0;
}

std::pair<uint8_t, uint8_t> PunchCard::_GetPunchAddr(uint8_t index)
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
