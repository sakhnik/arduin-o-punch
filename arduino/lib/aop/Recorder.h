#pragma once

#include <cstdint>
#include <vector>

namespace AOP {

class Recorder
{
public:
    Recorder();

    void Setup(uint32_t timestamp);

    void Record(uint16_t card, uint32_t timestamp);

    size_t GetCount() const;

    using TimestampsT = std::vector<uint32_t>;
    TimestampsT GetTimestamps(uint16_t card);

    struct IVisitor
    {
        virtual void OnCard(uint16_t card, uint32_t timestamp, void *ctx) = 0;
    };

    void List(IVisitor &, void *ctx);

private:
    size_t count = 0;
    uint32_t lastTimestamp;
};

} //namespace AOP;
