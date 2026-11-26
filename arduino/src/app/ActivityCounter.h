#pragma once

#include <atomic>

class ActivityCounter
{
    ActivityCounter()
        : counter(0)
    {
    }

public:
    static ActivityCounter& Instance();

    void Begin()
    {
        counter.fetch_add(1);
    }

    void End()
    {
        counter.fetch_sub(1);
    }

    bool IsBusy()
    {
        return counter.load() > 0;
    }

    class Scoped
    {
    public:
        Scoped(ActivityCounter &c = ActivityCounter::Instance())
            : counter(c)
        {
            counter.Begin();
        }

        ~Scoped()
        {
            counter.End();
        }

    private:
        ActivityCounter &counter;
    };

private:
    std::atomic<int> counter;
};
