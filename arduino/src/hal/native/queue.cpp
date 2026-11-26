#include "hal/queue.h"

#include <chrono>
#include <condition_variable>
#include <cstring>
#include <mutex>
#include <new>
#include <limits>

namespace HAL {

struct QueueImpl
{
    std::mutex mutex;
    std::condition_variable not_empty;
    std::condition_variable not_full;

    uint8_t *buffer = nullptr;

    size_t capacity;
    size_t item_size;
    size_t head = 0;
    size_t tail = 0;
    size_t count = 0;

    QueueImpl(size_t capacity, size_t item_size)
        : capacity(capacity), item_size(item_size)
    {
    }

    ~QueueImpl()
    {
        delete[] buffer;
    }
};

Queue::~Queue()
{
    Shutdown();
}

bool Queue::Setup(size_t capacity, size_t item_size)
{
    if (_handle || capacity == 0 || item_size == 0)
        return false;

    if (capacity > std::numeric_limits<size_t>::max() / item_size)
        return false;

    auto *impl = new (std::nothrow) QueueImpl(capacity, item_size);
    if (!impl)
        return false;

    impl->buffer = new (std::nothrow) uint8_t[capacity * item_size];
    if (!impl->buffer) {
        delete impl;
        return false;
    }

    _handle = impl;
    return true;
}

void Queue::Shutdown()
{
    delete static_cast<QueueImpl *>(_handle);
    _handle = nullptr;
}

bool Queue::Send(const void *item, uint32_t timeout_ms)
{
    if (!_handle || !item)
        return false;

    auto *impl = static_cast<QueueImpl *>(_handle);
    std::unique_lock<std::mutex> lock(impl->mutex);

    auto ready = [&] {
        return impl->count < impl->capacity;
    };

    if (timeout_ms == WaitForever) {
        impl->not_full.wait(lock, ready);
    } else if (!impl->not_full.wait_for(
                   lock, std::chrono::milliseconds(timeout_ms), ready)) {
        return false;
    }

    std::memcpy(
        impl->buffer + impl->tail * impl->item_size,
        item,
        impl->item_size
    );

    impl->tail = (impl->tail + 1) % impl->capacity;
    ++impl->count;

    lock.unlock();
    impl->not_empty.notify_one();

    return true;
}

bool Queue::Receive(void *item, uint32_t timeout_ms)
{
    if (!_handle || !item)
        return false;

    auto *impl = static_cast<QueueImpl *>(_handle);
    std::unique_lock<std::mutex> lock(impl->mutex);

    auto ready = [&] {
        return impl->count > 0;
    };

    if (timeout_ms == WaitForever) {
        impl->not_empty.wait(lock, ready);
    } else if (!impl->not_empty.wait_for(
                   lock, std::chrono::milliseconds(timeout_ms), ready)) {
        return false;
    }

    std::memcpy(
        item,
        impl->buffer + impl->head * impl->item_size,
        impl->item_size
    );

    impl->head = (impl->head + 1) % impl->capacity;
    --impl->count;

    lock.unlock();
    impl->not_full.notify_one();

    return true;
}

size_t Queue::Size() const
{
    if (!_handle)
        return 0;

    auto *impl = static_cast<QueueImpl *>(_handle);
    std::lock_guard<std::mutex> lock(impl->mutex);

    return impl->count;
}

} //namespace HAL;
