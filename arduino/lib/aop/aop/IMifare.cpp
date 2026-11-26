#include "IMifare.h"

namespace AOP {

const IMifare::KeyT IMifare::KEY_DEFAULT = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
const uint8_t IMifare::ACCESS_BITS[IMifare::ACCESS_BITS_SIZE] = {0xff, 0x07, 0x80};

} //namespace AOP;
