#include "hal/queue.h"

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

namespace Hal {

Queue::~Queue()
{
    Shutdown();
}

bool Queue::Setup(size_t capacity, size_t item_size)
{
    if (_handle || capacity == 0 || item_size == 0)
        return false;

    _handle = xQueueCreate(static_cast<UBaseType_t>(capacity), static_cast<UBaseType_t>(item_size));
    return _handle != nullptr;
}

void Queue::Shutdown()
{
    if (_handle) {
        vQueueDelete(static_cast<QueueHandle_t>(_handle));
        _handle = nullptr;
    }
}

bool Queue::Send(const void *item, uint32_t timeout_ms)
{
    if (!_handle || !item)
        return false;

    TickType_t timeout = timeout_ms == WaitForever
        ? portMAX_DELAY
        : pdMS_TO_TICKS(timeout_ms);

    if (timeout_ms != 0 && timeout_ms != WaitForever && timeout == 0)
        timeout = 1;

    return xQueueSend(static_cast<QueueHandle_t>(_handle), item, timeout) == pdTRUE;
}

bool Queue::Receive(void *item, uint32_t timeout_ms)
{
    if (!_handle || !item)
        return false;

    TickType_t timeout = timeout_ms == WaitForever
        ? portMAX_DELAY
        : pdMS_TO_TICKS(timeout_ms);

    if (timeout_ms != 0 && timeout_ms != WaitForever && timeout == 0)
        timeout = 1;

    return xQueueReceive(static_cast<QueueHandle_t>(_handle), item, timeout) == pdTRUE;
}

size_t Queue::Size() const
{
    if (!_handle)
        return 0;

    return uxQueueMessagesWaiting(static_cast<QueueHandle_t>(_handle));
}

}
