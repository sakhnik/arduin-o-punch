#include "ActivityCounter.hpp"

ActivityCounter& ActivityCounter::Instance()
{
    static ActivityCounter instance;
    return instance;
}
