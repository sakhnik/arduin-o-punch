#include "ActivityCounter.h"

ActivityCounter& ActivityCounter::Instance()
{
    static ActivityCounter instance;
    return instance;
}
