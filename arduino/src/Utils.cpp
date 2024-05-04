#include "Utils.h"

char* PrintNum(uint8_t num, char *buf)
{
    if (!num)
        return buf;
    buf = PrintNum(num / 10, buf);
    *buf = '0' + num % 10;
    return ++buf;
}