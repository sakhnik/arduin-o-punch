#pragma once

#include "src/IKeyReceiver.h"
#include "src/Recorder.h"
#include <Arduino.h>
#include <RTClib.h>

class Buzzer;

class Context
    : public AOP::IKeyReceiver
{
public:
    static constexpr const int ADDRESS = 0;

    Context(Buzzer &buzzer);

    int8_t Setup();

    static const uint8_t KEY_SIZE = 6;
    const uint8_t* GetKey() const { return _key; }
    bool IsKeyDefault() const;

    // IKeyReceiver
    void OnNewKey(const uint8_t *key) override;

    DateTime GetDateTime() const;
    uint32_t GetClock(const DateTime *date_time) const;
    void SetClock(uint32_t clock);

    uint8_t GetId() const { return _id; }
    void SetId(uint8_t id);

    uint8_t GetTimeoutHours() const { return _timeout_hr; }
    void SetTimeoutHours(uint8_t hours);

    AOP::Recorder& GetRecorder() { return _recorder; }

private:
    Buzzer &_buzzer;
    uint8_t _id = 1;
    uint8_t _key[KEY_SIZE] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
    uint8_t _timeout_hr = 3;
    uint8_t _reserved[8];
    AOP::Recorder _recorder;

    friend struct RecorderAccess;
};
