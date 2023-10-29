#pragma once

#include <Arduino.h>
#include "src/IKeyReceiver.h"

class Context
    : public AOP::IKeyReceiver
{
public:
    static constexpr const int ADDRESS = 0;

    void Setup();

    static const uint8_t KEY_SIZE = 6;
    const uint8_t* GetKey() const { return _key; }
    bool IsKeyDefault() const;

    // IKeyReceiver
    void OnNewKey(const uint8_t *key) override;

    uint32_t GetClock() const;
    void SetClock(uint32_t clock);

    uint8_t GetId() const;
    void SetId(uint8_t id);

private:
    uint8_t _id = 1;
    uint8_t _key[KEY_SIZE] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
    long _clock_offset = 0;
};
