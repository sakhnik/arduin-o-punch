#include "hal/serial.hpp"

#include <cstdlib>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

namespace HAL {

static termios old_termios;

static void RestoreTerminal()
{
    tcsetattr(STDIN_FILENO, TCSANOW, &old_termios);
}

void Serial::Setup()
{
    tcgetattr(STDIN_FILENO, &old_termios);

    termios t = old_termios;
    t.c_lflag &= ~(ICANON | ECHO);
    t.c_cc[VMIN] = 1;
    t.c_cc[VTIME] = 0;

    tcsetattr(STDIN_FILENO, TCSANOW, &t);

    std::atexit(RestoreTerminal);
}

size_t Serial::Write(uint8_t byte)
{
    return ::write(STDOUT_FILENO, &byte, 1) == 1 ? 1 : 0;
}

size_t Serial::Write(const uint8_t *buffer, size_t size)
{
    return ::write(STDOUT_FILENO, buffer, size);
}

bool Serial::AvailableForWrite()
{
    return true;
}

bool Serial::Available()
{
    fd_set set;
    FD_ZERO(&set);
    FD_SET(STDIN_FILENO, &set);

    timeval timeout{};
    return select(STDIN_FILENO + 1, &set, nullptr, nullptr, &timeout) > 0;
}

char Serial::Read()
{
    char c;
    return ::read(STDIN_FILENO, &c, 1) == 1 ? c : '\0';
}

} // namespace HAL
