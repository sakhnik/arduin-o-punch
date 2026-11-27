#pragma once

#include <stddef.h>
#include <stdint.h>

namespace HAL {

class Queue
{
public:
    Queue() = default;
    ~Queue();

    Queue(const Queue &) = delete;
    Queue &operator=(const Queue &) = delete;

    bool Setup(size_t capacity, size_t item_size);
    void Shutdown();

    static constexpr const uint32_t WaitForever = UINT32_MAX;

    bool Send(const void *item, uint32_t timeout_ms = 0);
    bool Receive(void *item, uint32_t timeout_ms = WaitForever);

    size_t Size() const;

private:
    void *_handle = nullptr;
};

} //namespace HAL;
