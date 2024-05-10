#pragma once

#include <Arduino.h>

class OutMux : public HardwareSerial
{
public:
    OutMux();

    struct IClient
    {
        virtual void Write(const uint8_t *buffer, size_t size) = 0;
    };

    void SetClient(IClient *client)
    {
        _client = client;
    }

private:
    IClient *_client = nullptr;
    bool _enable_serial = true;

    size_t write(uint8_t) override;
    size_t write(const uint8_t *buffer, size_t size) override;
    int availableForWrite(void) override;
};
