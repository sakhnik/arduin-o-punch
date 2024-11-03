#include "IMifare.h"

namespace AOP {

const uint8_t IMifare::KEY_DEFAULT[IMifare::KEY_SIZE] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
const uint8_t IMifare::ACCESS_BITS[IMifare::ACCESS_BITS_SIZE] = {0xff, 0x07, 0x80};

} //namespace AOP;
