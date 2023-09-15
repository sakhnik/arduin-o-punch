
#include "Punch.h"
#include "IMifare.h"
#include <utility>
#include <vector>

namespace AOP {

class PunchCard
{
    static constexpr uint8_t HEADER_BLOCK = 1;

    IMifare *_mifare;
    const uint8_t *_key;
    int _auth_sector = -1;

public:
    PunchCard(IMifare *, const uint8_t *key);

    static void DummyProgress(int, int) { }

    int Punch(Punch punch, void(*progress)(int, int) = &DummyProgress);
    int ReadOut(std::vector<AOP::Punch> &punches, void (*progress)(int, int) = &DummyProgress);

private:
    int _Authenticate(uint8_t sector);
    std::pair<uint8_t, uint8_t> _GetPunchAddr(uint8_t index);
};

} //namespace AOP;
