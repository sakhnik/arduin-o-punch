#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

namespace HAL {

class Prefs
{
public:
    Prefs(const char *name, bool readOnly);
    ~Prefs();

    Prefs(const Prefs &) = delete;
    Prefs& operator=(const Prefs &) = delete;

    uint8_t GetU8(const char *key, uint8_t def);
    uint16_t GetU16(const char *key, uint16_t def);
    void GetBytes(const char *key, uint8_t *data, size_t size);
    size_t GetBytesLength(const char *key);
    std::string GetString(const char *key);

    void PutU8(const char *key, uint8_t val);
    void PutU16(const char *key, uint16_t val);
    void PutBytes(const char *key, const uint8_t *data, size_t size);
    void PutString(const char *key, const char *val);

private:
    const char *name;
    bool readOnly;
};

} //namespace HAL;
