#include "hal/mutex.h"
#include <mutex>

namespace HAL {

struct Impl
{
    std::mutex mutex;
};

Mutex::Mutex()
    : _impl{new Impl()}
{
}

Mutex::~Mutex()
{
    delete static_cast<Impl *>(_impl);
}

void Mutex::lock()
{
    static_cast<Impl *>(_impl)->mutex.lock();
}

void Mutex::unlock()
{
    static_cast<Impl *>(_impl)->mutex.unlock();
}

} //namespace HAL;
