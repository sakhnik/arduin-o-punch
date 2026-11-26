#include "hal/task.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

#include <atomic>
#include <new>

namespace Hal {

struct TaskImpl
{
    TaskHandle_t handle = nullptr;
    SemaphoreHandle_t done = nullptr;

    std::atomic<bool> stop_requested{false};
    std::atomic<bool> running{false};

    Task::TaskEntry entry;
    void *arg;
};

static void TaskTrampoline(void *arg)
{
    auto *impl = static_cast<TaskImpl *>(arg);

    impl->entry(impl->arg);

    impl->running.store(false, std::memory_order_release);
    xSemaphoreGive(impl->done);

    vTaskDelete(nullptr);
}

Task::~Task()
{
    Shutdown();
}

bool Task::Setup(TaskEntry entry, void *arg, const char *name, size_t stack_size, int priority)
{
    if (_handle || !entry)
        return false;

    auto *impl = new (std::nothrow) TaskImpl;
    if (!impl)
        return false;

    impl->entry = entry;
    impl->arg = arg;
    impl->done = xSemaphoreCreateBinary();

    if (!impl->done) {
        delete impl;
        return false;
    }

    impl->running.store(true, std::memory_order_relaxed);

    // Publish before the task can start executing.
    _handle = impl;

    BaseType_t result = xTaskCreate(
        TaskTrampoline,
        name,
        stack_size,
        impl,
        priority,
        &impl->handle
    );

    if (result != pdPASS) {
        _handle = nullptr;
        vSemaphoreDelete(impl->done);
        delete impl;
        return false;
    }

    return true;
}

void Task::Shutdown()
{
    auto *impl = static_cast<TaskImpl *>(_handle);
    if (!impl)
        return;

    impl->stop_requested.store(true, std::memory_order_release);

    // Wait until the task has returned.
    xSemaphoreTake(impl->done, portMAX_DELAY);

    vSemaphoreDelete(impl->done);
    delete impl;

    _handle = nullptr;
}

bool Task::IsRunning() const
{
    auto *impl = static_cast<TaskImpl *>(_handle);
    return impl && impl->running.load(std::memory_order_acquire);
}

} //namespace Hal;
