#pragma once

#include "../Printer.hpp"
#include <cstddef>
#include <cstdint>

class OutMux
    : public Printer<OutMux>
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

    size_t Write(uint8_t);
    size_t Write(const uint8_t *buffer, size_t size);
    bool AvailableForWrite(void);

private:
    IClient *_client = nullptr;
    bool _enable_serial = true;
};
