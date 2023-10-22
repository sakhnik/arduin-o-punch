#pragma once

#include <Arduino.h>
#include "src/IKeyReceiver.h"

class Context
    : public AOP::IKeyReceiver
{
public:
    const uint8_t* GetDefaultKey() const { return reinterpret_cast<const uint8_t *>(F("\xff\xff\xff\xff\xff\xff")); }
    bool IsKeyInitialized() const { return _key_initialized; }
    const uint8_t* GetKey() const { return _key_initialized ? _key : GetDefaultKey(); }

    void SetKeyHex(const char *key);

    // IKeyReceiver
    void OnNewKey(const uint8_t *key) override;

private:
    uint8_t _key[6] = {};
    bool _key_initialized = false;
};
