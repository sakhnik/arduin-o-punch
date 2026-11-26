#include "hal/mutex.h"

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

namespace HAL {

struct Impl
{
    SemaphoreHandle_t handle;
};

Mutex::Mutex()
    : _impl{new Impl()}
{
    auto *impl = static_cast<Impl *>(_impl);
    impl->handle = xSemaphoreCreateMutex();
    configASSERT(impl->handle);
}

Mutex::~Mutex()
{
    auto *impl = static_cast<Impl *>(_impl);
    vSemaphoreDelete(impl->handle);
    delete impl;
}

void Mutex::lock()
{
    auto *impl = static_cast<Impl *>(_impl);
    xSemaphoreTake(impl->handle, portMAX_DELAY);
}

void Mutex::unlock()
{
    auto *impl = static_cast<Impl *>(_impl);
    xSemaphoreGive(impl->handle);
}

} //namespace HAL;
