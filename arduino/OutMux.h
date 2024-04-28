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

    size_t write(const uint8_t *buffer, size_t size) override;
};
