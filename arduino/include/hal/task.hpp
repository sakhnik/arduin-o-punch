#pragma once

#include <cstddef>

namespace HAL {

class Task
{
public:
    Task() = default;
    ~Task();

    Task(const Task &) = delete;
    Task &operator=(const Task &) = delete;

    using TaskEntry = void(*)(void *);

    bool Setup(TaskEntry entry, void *arg, const char *name, size_t stack_size, int priority);

    void Shutdown();

    bool IsRunning() const;

    static void Yield();

private:
    void *_handle = nullptr;
};

} //namespace HAL;
