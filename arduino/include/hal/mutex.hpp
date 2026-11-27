#pragma once

namespace HAL {

class Mutex
{
public:
    Mutex();
    ~Mutex();

    Mutex(const Mutex &) = delete;
    Mutex& operator=(const Mutex &) = delete;

    void lock();
    void unlock();

private:
    void *_impl = nullptr;
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

} //namespace HAL;
