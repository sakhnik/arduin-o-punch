
#include "Punch.h"
#include "IMifare.h"

#ifdef BUILD_TEST
# include <vector>
#endif

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
#ifdef BUILD_TEST
    int ReadOut(std::vector<AOP::Punch> &punches, void (*progress)(int, int) = &DummyProgress);
#endif

private:
    int _Authenticate(uint8_t sector);

    struct _Address
    {
        uint8_t block;
        uint8_t offset;
    };
    _Address _GetPunchAddr(uint8_t index);
};

} //namespace AOP;
