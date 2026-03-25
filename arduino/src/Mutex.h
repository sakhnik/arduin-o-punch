#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

class Mutex
{
public:
    Mutex()
    {
        handle = xSemaphoreCreateMutex();
        configASSERT(handle);
    }

    ~Mutex()
    {
        vSemaphoreDelete(handle);
    }

    void lock()
    {
        xSemaphoreTake(handle, portMAX_DELAY);
    }

    void unlock()
    {
        xSemaphoreGive(handle);
    }

    SemaphoreHandle_t native() const { return handle; }

private:
    SemaphoreHandle_t handle;
};

class LockGuard
{
public:
    explicit LockGuard(Mutex &m)
        : mutex(m)
    {
        mutex.lock();
    }

    ~LockGuard()
    {
        mutex.unlock();
    }

    LockGuard(const LockGuard &) = delete;
    LockGuard& operator=(const LockGuard &) = delete;

private:
    Mutex &mutex;
};
