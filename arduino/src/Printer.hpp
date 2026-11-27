#pragma once

#include "hal/serial.hpp"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <type_traits>
#include <utility>
#ifdef ARDUINO
#include <WString.h>
#endif

#ifndef ARDUINO
struct __FlashStringHelper {};
#define F(str) (reinterpret_cast<const __FlashStringHelper *>(str))
#endif //ARDUINO

template <typename T>
class Printer
{
public:
    size_t Print(const char *str)
    {
        return Write(reinterpret_cast<const uint8_t *>(str),
                     std::strlen(str));
    }

    size_t Print(const __FlashStringHelper *str)
    {
        return Print(reinterpret_cast<const char *>(str));
    }

    size_t Print(char c)
    {
        return Write(static_cast<uint8_t>(c));
    }

    size_t Print(bool value)
    {
        return Print(value ? "true" : "false");
    }

    size_t Print(int value)
    {
        return PrintInteger(value);
    }

    size_t Print(unsigned value)
    {
        return PrintInteger(value);
    }

    size_t Print(long value)
    {
        return PrintInteger(value);
    }

    size_t Print(unsigned long value)
    {
        return PrintInteger(value);
    }

    size_t Print(long long value)
    {
        return PrintInteger(value);
    }

    size_t Print(unsigned long long value)
    {
        return PrintInteger(value);
    }

    size_t Print(long value, uint8_t base)
    {
        if (base < 2 || base > 16)
            base = 10;

        char buffer[sizeof(value) * 8 + 1];
        char *end = buffer + sizeof(buffer);
        char *pos = end;

        bool negative = value < 0 && base == 10;
        unsigned long n = negative
            ? 0UL - static_cast<unsigned long>(value)
            : static_cast<unsigned long>(value);

        do {
            unsigned digit = n % base;
            *--pos = digit < 10 ? '0' + digit : 'A' + digit - 10;
            n /= base;
        } while (n);

        if (negative)
            *--pos = '-';

        return Write(reinterpret_cast<const uint8_t *>(pos), end - pos);
    }

    //size_t Print(float value, int precision = 2)
    //{
    //    char buffer[32];
    //    int len = std::snprintf(buffer, sizeof(buffer), "%.*f",
    //                            precision, static_cast<double>(value));
    //    return len > 0
    //        ? Write(reinterpret_cast<const uint8_t *>(buffer),
    //                len < sizeof(buffer) ? len : sizeof(buffer) - 1)
    //        : 0;
    //}

    template <typename... Args>
    size_t Println(Args&&... args)
    {
        size_t written = (Print(std::forward<Args>(args)) + ... + 0);
        return written + Print("\r\n");
    }

    size_t Println()
    {
        return Print("\r\n");
    }

private:
    size_t Write(uint8_t byte)
    {
        return static_cast<T *>(this)->Write(byte);
    }

    size_t Write(const uint8_t *buffer, size_t size)
    {
        return static_cast<T *>(this)->Write(buffer, size);
    }

    template <typename TNumber>
    size_t PrintInteger(TNumber value)
    {
        char buffer[32];
        char *end = buffer + sizeof(buffer);
        char *pos = end;

        using Unsigned = std::make_unsigned_t<TNumber>;
        Unsigned n;

        if constexpr (std::is_signed_v<TNumber>) {
            if (value < 0) {
                n = Unsigned(0) - static_cast<Unsigned>(value);
            } else {
                n = static_cast<Unsigned>(value);
            }
        } else {
            n = value;
        }

        do {
            *--pos = '0' + n % 10;
            n /= 10;
        } while (n);

        if constexpr (std::is_signed_v<TNumber>) {
            if (value < 0)
                *--pos = '-';
        }

        return Write(reinterpret_cast<const uint8_t *>(pos), end - pos);
    }
};

struct SerialOutput : Printer<SerialOutput>
{
    size_t Write(uint8_t byte)
    {
        return HAL::Serial::Write(byte);
    }

    size_t Write(const uint8_t *buffer, size_t size)
    {
        return HAL::Serial::Write(buffer, size);
    }
};

inline SerialOutput OUT;
