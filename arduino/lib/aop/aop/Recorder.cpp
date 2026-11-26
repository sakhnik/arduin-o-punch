#include "Recorder.h"
#include <cstddef>
#include <cstring>

namespace AOP {

constexpr size_t MAX_PUNCHES = 10000;

struct PunchRecord
{
    uint16_t card;
    uint16_t delta_seconds;
};

static_assert(sizeof(PunchRecord) == 4);

PunchRecord punches[MAX_PUNCHES] = {};

Recorder::Recorder()
{
}

void Recorder::Setup(uint32_t timestamp)
{
    lastTimestamp = timestamp;
    count = 0;
}

void Recorder::Record(uint16_t card, uint32_t timestamp)
{
    auto idx = count % MAX_PUNCHES;
    punches[idx].card = card;
    punches[idx].delta_seconds = timestamp - lastTimestamp;
    ++count;
    lastTimestamp = timestamp;
}

size_t Recorder::GetCount() const
{
    return std::min(MAX_PUNCHES, count);
}

Recorder::TimestampsT Recorder::GetTimestamps(uint16_t card)
{
    TimestampsT timestamps;

    struct Visitor : IVisitor
    {
        uint16_t card_;
        TimestampsT &timestamps_;

        Visitor(uint16_t card, TimestampsT &timestamps)
            : card_{card}
            , timestamps_{timestamps}
        {
        }

        void OnCard(uint16_t card, uint32_t timestamp, void *ctx) override
        {
            if (card == card_)
                timestamps_.push_back(timestamp);
        }
    } visitor{card, timestamps};

    List(visitor, nullptr);

    return timestamps;
}

void Recorder::List(IVisitor &visitor, void *ctx)
{
    auto idx = (count + MAX_PUNCHES - 1) % MAX_PUNCHES;
    auto timestamp = lastTimestamp;
    for (uint32_t i = 0, n = GetCount(); i != n; ++i) {
        visitor.OnCard(punches[idx].card, timestamp, ctx);
        timestamp -= punches[idx].delta_seconds;
        idx = (idx + MAX_PUNCHES - 1) % MAX_PUNCHES;
    }
}

} //namespace AOP;
