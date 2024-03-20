#include "PunchCard.h"
#include "ErrorCode.h"

#if defined(BUILD_TEST) || defined(BUILD_WA)
# include <cstring>
#else
# include <Arduino.h>
#endif

namespace AOP {

PunchCard::PunchCard(IMifare *mifare, const uint8_t *key, ICallback *callback)
    : _mifare{mifare}
    , _key{key}
    , _callback{callback}
{
}

ErrorCode PunchCard::Punch(AOP::Punch punch)
{
    // 1. read card id
    auto headerSector = _mifare->BlockToSector(INDEX_KEY_BLOCK);
    if (auto res = _Authenticate(headerSector))
        return res;
    uint8_t header[IMifare::BLOCK_SIZE + 2];  // need at least 18 bytes +crc
    uint8_t headerSize = sizeof(header);
    if (auto res = _mifare->ReadBlock(INDEX_KEY_BLOCK, header, headerSize))
        return res;
    uint16_t card_id = static_cast<uint16_t>(header[ID_OFFSET]) | (static_cast<uint16_t>(header[ID_OFFSET + 1]) << 8);
    uint8_t startSector = header[SECTOR_OFFSET];

    auto headerStatus = _RecoverHeader(startSector, header);
    if (headerStatus.error)
        return headerStatus.error;

    // 5. calculate the offset in the header
    // If this is the start station punching, clear all the previous punches
    if (punch.GetStation() == START_STATION) {
        memset(header, 0xff, IMifare::BLOCK_SIZE);
        header[0] = 0;
    }
    uint8_t count = header[0];
    uint8_t offsetInBlock = count % PUNCHES_PER_BLOCK;

    //   a. check the station is different and not the start station.
    // allow punching the start station many times because the index is reset

    int prevStationId = !count
                        ? -1
                        : Punch::GetStation(header, STATION_OFFSET + (count - 1) % PUNCHES_PER_BLOCK);

    if (punch.GetStation() != START_STATION && prevStationId == punch.GetStation()) {
        if (_callback)
            _callback->OnCardId(card_id);
        // If the previous punch hasn't been confirmed (header2 succeeded, header1 failed), it can
        // still be recovered from the header2. Confirm it now, it isn't a duplicate.
        return headerStatus.status == HeaderStatus::UNCONFIRMED ? ErrorCode::OK : ErrorCode::DUPLICATE_PUNCH;
    }

    if (count == GetMaxPunches()) {
        return ErrorCode::CARD_IS_FULL;
    }

    // 6. if the offset is 4, store the completed row to the proper block
    if (count != 0 && !offsetInBlock) {
        auto block = _GetPunchBlock(count - PUNCHES_PER_BLOCK, startSector);
        if (auto res = _Authenticate(_mifare->BlockToSector(block)))
            return res;
        if (auto res = _mifare->WriteBlock(block, header, IMifare::BLOCK_SIZE))
            return res;
        memset(header + STATION_OFFSET, 0xff, IMifare::BLOCK_SIZE - STATION_OFFSET);
    }
    ++header[COUNT_OFFSET];

    // 7. add data to the header
    uint8_t blockOffset = count % PUNCHES_PER_BLOCK;
    punch.SerializeStation(header, STATION_OFFSET + blockOffset);
    if (!blockOffset) {
        // Store full timestamp for the first punch in the block
        punch.SerializeTimestamp(header, TIME1_OFFSET);
    } else {
        // Store two-byte difference from the first punch timestamp in the block
        auto timestamp0 = Punch::GetTimestamp(header, TIME1_OFFSET);
        punch.SerializeTimestamp(header, TIME2_OFFSET + (blockOffset - 1) * 2, timestamp0);
    }

    // 8. write header 2
    auto headerBlock2 = startSector * IMifare::BLOCKS_PER_SECTOR + HEADER_BLOCK2;
    if (auto res = _Authenticate(startSector))
        return res;
    if (auto res = _mifare->WriteBlock(headerBlock2, header, IMifare::BLOCK_SIZE))
        return res;

    // 9. write header 1
    auto headerBlock1 = startSector * IMifare::BLOCKS_PER_SECTOR + HEADER_BLOCK1;
    if (auto res = _Authenticate(startSector))
        return res;
    if (auto res = _mifare->WriteBlock(headerBlock1, header, IMifare::BLOCK_SIZE))
        return res;

    if (_callback)
        _callback->OnCardId(card_id);
    return ErrorCode::OK;
}

ErrorCode PunchCard::Clear()
{
    // 1. read startSector
    auto indexSector = _mifare->BlockToSector(INDEX_KEY_BLOCK);
    if (auto res = _Authenticate(indexSector))
        return res;
    uint8_t header[IMifare::BLOCK_SIZE + 2];  // need at least 18 bytes +crc
    uint8_t headerSize = sizeof(header);
    if (auto res = _mifare->ReadBlock(INDEX_KEY_BLOCK, header, headerSize))
        return res;
    uint8_t startSector = header[SECTOR_OFFSET];

    if (auto res = _ClearPunches(startSector)) {
        return res;
    }

    return ErrorCode::OK;
}

uint8_t PunchCard::_ClearPunches(uint8_t startSector)
{
    if (auto res = _Authenticate(startSector))
        return res;
    uint8_t headerBlock1 = startSector * IMifare::BLOCKS_PER_SECTOR + HEADER_BLOCK1;
    uint8_t headerBlock2 = startSector * IMifare::BLOCKS_PER_SECTOR + HEADER_BLOCK2;
    uint8_t header[IMifare::BLOCK_SIZE + 2];
    memset(header, 0xff, IMifare::BLOCK_SIZE);
    header[0] = 0;
    if (auto res = _mifare->WriteBlock(headerBlock2, header, IMifare::BLOCK_SIZE))
        return res;
    if (auto res = _mifare->WriteBlock(headerBlock1, header, IMifare::BLOCK_SIZE))
        return res;
    return 0;
}

#if defined(BUILD_TEST) || defined(BUILD_WA)

ErrorCode PunchCard::Format(uint16_t id, uint8_t startSector)
{
    if (startSector >= _mifare->SECTOR_COUNT) {
        startSector = rand() % _mifare->SECTOR_COUNT;
    }

    for (int sector = 0; sector < _mifare->SECTOR_COUNT; ++sector) {
        if (auto res = _mifare->AuthenticateSectorWithKeyA(sector, IMifare::KEY_DEFAULT)) {
            return res;
        }
        _auth_sector = sector;
        uint8_t blockIndex = IMifare::BLOCKS_PER_SECTOR * (sector + 1) - 1;
        uint8_t trailer[IMifare::BLOCK_SIZE + 2];
        uint8_t dataSize = sizeof(trailer);
        if (auto res = _mifare->ReadBlock(blockIndex, trailer, dataSize)) {
            return res;
        }
        memcpy(trailer, _key, IMifare::KEY_SIZE);
        // Restore default access bits to use KeyB for data
        memcpy(trailer + 6, IMifare::ACCESS_BITS, IMifare::ACCESS_BITS_SIZE);
        // Write card ID to KeyB
        trailer[ID_OFFSET] = id & 0xff;
        trailer[ID_OFFSET + 1] = (id >> 8) & 0xff;
        trailer[SECTOR_OFFSET] = startSector;
        if (auto res = _mifare->WriteBlock(blockIndex, trailer, IMifare::BLOCK_SIZE)) {
            return res;
        }
    }

    if (auto res = _ClearPunches(startSector)) {
        return res;
    }

    return ErrorCode::OK;
}

uint8_t PunchCard::ReadOut(std::vector<AOP::Punch> &punches)
{
    // 1. read card id
    auto headerSector = _mifare->BlockToSector(INDEX_KEY_BLOCK);
    if (auto res = _Authenticate(headerSector))
        return res;
    uint8_t header[IMifare::BLOCK_SIZE + 2];  // need at least 18 bytes +crc
    uint8_t headerSize = sizeof(header);
    if (auto res = _mifare->ReadBlock(INDEX_KEY_BLOCK, header, headerSize))
        return res;
    //uint16_t card_id = static_cast<uint16_t>(header[ID_OFFSET]) | (static_cast<uint16_t>(header[ID_OFFSET + 1]) << 8);
    uint8_t startSector = header[SECTOR_OFFSET];

    // 2. Recover the header
    auto headerStatus = _RecoverHeader(startSector, header);
    if (headerStatus.error)
        return headerStatus.error;
    uint8_t count = header[COUNT_OFFSET];

    // 5. loop over the punches
    punches.clear();
    if (count > 0) {
        punches.reserve(count);
        uint8_t tail = count % PUNCHES_PER_BLOCK;
        if (tail == 0) {
            tail = PUNCHES_PER_BLOCK;
        }
        uint8_t wholeBlocks = (count - tail) / PUNCHES_PER_BLOCK;
        for (int i = 0; i < wholeBlocks; ++i) {
            uint8_t block = _GetPunchBlock(i * PUNCHES_PER_BLOCK, startSector);
            if (auto res = _Authenticate(_mifare->BlockToSector(block)))
                return res;
            uint8_t data[IMifare::BLOCK_SIZE + 2];  // need at least 18 bytes +crc
            uint8_t dataSize = sizeof(data);
            if (auto res = _mifare->ReadBlock(block, data, dataSize))
                return res;
            _ReadPunchesFromBlock(PUNCHES_PER_BLOCK, data, punches);
        }
        _ReadPunchesFromBlock(tail, header, punches);
    }
    return 0;
}

void PunchCard::_ReadPunchesFromBlock(uint8_t count, const uint8_t *data, PunchesT &punches)
{
    uint32_t timestamp0 = 0;
    for (uint8_t offset = 0; offset != count; ++offset) {
        uint8_t stationId = Punch::GetStation(data, STATION_OFFSET + offset);
        if (!offset) {
            timestamp0 = Punch::GetTimestamp(data, TIME1_OFFSET);
            punches.emplace_back(stationId, timestamp0);
        } else {
            auto timestamp = Punch::GetTimestamp(data, TIME2_OFFSET + 2 * (offset - 1), timestamp0);
            punches.emplace_back(stationId, timestamp);
        }
    }
}

#endif //BUILD_TEST

uint8_t PunchCard::_Authenticate(uint8_t sector)
{
    if (sector != _auth_sector) {
        if (auto res = _mifare->AuthenticateSectorWithKeyA(sector, _key))
            return res;
        _auth_sector = sector;
    }
    return 0;
}

PunchCard::HeaderStatus PunchCard::_RecoverHeader(uint8_t startSector, uint8_t *header)
{
    uint8_t headerBlock1 = startSector * IMifare::BLOCKS_PER_SECTOR + HEADER_BLOCK1;
    uint8_t headerBlock2 = startSector * IMifare::BLOCKS_PER_SECTOR + HEADER_BLOCK2;

    // 2. read header 1
    if (auto res = _Authenticate(startSector))
        return {res};

    uint8_t header1[IMifare::BLOCK_SIZE + 2];
    uint8_t header1Size = sizeof(header1);
    if (uint8_t res = _mifare->ReadBlock(headerBlock1, header1, header1Size))
        return {res};

    // 3. read header 2
    uint8_t header2[IMifare::BLOCK_SIZE + 2];
    uint8_t header2Size = sizeof(header2);
    if (uint8_t res = _mifare->ReadBlock(headerBlock2, header2, header2Size))
        return {res};

    // 4. decide the right one
    // 4.1 write the missing one back
    uint8_t header1IsOk = header1[0] != 0xff;
    uint8_t header2IsOk = header2[0] != 0xff;
    if (!header1IsOk && !header2IsOk) {
        return {ErrorCode::DATA_CORRUPTED};
    }
    // Header2 is written first, header1 is written second.
    // If there's just header2, it contains an unconfirmed punch.
    // If there's header1, there should also be header2 because it was written first.
    if (header1IsOk) {
        memcpy(header, header1, IMifare::BLOCK_SIZE);
        // Store the chosen copy to the other header block if required
        if (header1[0] != header2[0]) {
            if (auto res = _mifare->WriteBlock(headerBlock2, header1, IMifare::BLOCK_SIZE))
                return {res};
        }
    } else {
        memcpy(header, header2, IMifare::BLOCK_SIZE);
        // Store the chosen copy to the other header block if required
        if (header1[0] != header2[0]) {
            if (auto res = _mifare->WriteBlock(headerBlock1, header2, IMifare::BLOCK_SIZE))
                return {res};
            return {ErrorCode::OK, HeaderStatus::UNCONFIRMED};
        }
    }
    return {ErrorCode::OK, HeaderStatus::CONFIRMED};
}

uint8_t PunchCard::_GetPunchBlock(uint8_t index, uint8_t startSector)
{
    uint8_t sector = (index / PUNCHES_PER_SECTOR + startSector + 1);
    auto sectorOffset = index % PUNCHES_PER_SECTOR / PUNCHES_PER_BLOCK;
    if (sector >= IMifare::SECTOR_COUNT) {
        sector -= IMifare::SECTOR_COUNT;
        // Block 0 isn't writable, shift everything down.
        if (++sectorOffset == IMifare::BLOCKS_PER_SECTOR - 1) {
            sectorOffset = 0;
            ++sector;
        }
    }
    return sector * IMifare::BLOCKS_PER_SECTOR + sectorOffset;
}

}  // namespace AOP;
