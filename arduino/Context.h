#pragma once

#include <Arduino.h>
#include "src/IKeyReceiver.h"

class Context
    : public AOP::IKeyReceiver
{
public:
    static const uint8_t KEY_SIZE = 6;

    const uint8_t* GetKey() const { return _key; }

    // IKeyReceiver
    void OnNewKey(const uint8_t *key) override;

private:
    uint8_t _key[KEY_SIZE] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
};