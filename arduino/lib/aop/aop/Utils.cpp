#include "Utils.h"

namespace AOP {

char* PrintNum(unsigned num, char *buf)
{
    if (!num)
        return buf;
    buf = PrintNum(num / 10, buf);
    *buf = '0' + num % 10;
    return ++buf;
}

} //namespace AOP;
