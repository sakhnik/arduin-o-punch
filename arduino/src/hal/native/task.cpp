#include "hal/task.h"

#include <pthread.h>

#include <atomic>
#include <algorithm>
#include <new>

namespace HAL {

struct TaskImpl {
    pthread_t thread{};

    std::atomic<bool> stop_requested{false};
    std::atomic<bool> running{false};

    Task::TaskEntry entry;
    void *arg;
};

static void *TaskTrampoline(void *arg)
{
    auto *impl = static_cast<TaskImpl *>(arg);

    impl->entry(impl->arg);

    impl->running.store(false, std::memory_order_release);

    return nullptr;
}

Task::~Task()
{
    Shutdown();
}

bool Task::Setup(
    TaskEntry entry,
    void *arg,
    const char *name,
    size_t stack_size,
    int priority)
{
    if (_handle || !entry)
        return false;

    (void)name;
    (void)priority;

    auto *impl = new (std::nothrow) TaskImpl;
    if (!impl)
        return false;

    impl->entry = entry;
    impl->arg = arg;
    impl->running.store(true, std::memory_order_relaxed);

    pthread_attr_t attr;

    if (pthread_attr_init(&attr) != 0) {
        delete impl;
        return false;
    }

    if (stack_size != 0) {
        stack_size = std::max(
            stack_size,
            static_cast<size_t>(PTHREAD_STACK_MIN));

        if (pthread_attr_setstacksize(&attr, stack_size) != 0) {
            pthread_attr_destroy(&attr);
            delete impl;
            return false;
        }
    }

    // Publish before the thread starts executing.
    _handle = impl;

    int result = pthread_create(
        &impl->thread,
        &attr,
        TaskTrampoline,
        impl);

    pthread_attr_destroy(&attr);

    if (result != 0) {
        _handle = nullptr;
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

    pthread_join(impl->thread, nullptr);

    delete impl;
    _handle = nullptr;
}

bool Task::IsRunning() const
{
    auto *impl = static_cast<TaskImpl *>(_handle);
    return impl && impl->running.load(std::memory_order_acquire);
}

} //namespace HAL;
