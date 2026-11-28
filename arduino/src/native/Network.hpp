#pragma once

#include "../app/OutMux.hpp"
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <thread>

namespace httplib {
class Server;
struct Request;
struct Response;
} // namespace httplib

class Settings;
class Shell;
class Buzzer;

namespace Native {

class Network
    : public OutMux::IClient
{
public:
    Network(OutMux &, Settings &, Shell &, Buzzer &);
    ~Network();

    Network(const Network &) = delete;
    Network &operator=(const Network &) = delete;

    void Setup();
    void SwitchOn();
    void SwitchOff();
    unsigned GetInactivitySeconds();

private:
    OutMux &_outMux;
    Settings &_settings;
    Shell &_shell;
    Buzzer &_buzzer;

    std::unique_ptr<httplib::Server> _webServer;
    std::thread _httpThread;
    std::thread _shellThread;

    std::atomic<bool> _running{false};
    std::atomic<uint32_t> _lastActivityTimeMs{0};
    std::atomic<int> _clientFd{-1};

    std::mutex _mutex;      // serialises Shell/Settings access (HTTP thread vs shell thread)
    std::mutex _writeMutex; // guards shell socket writes/close

    void _ShellTask();

    void _HandleGetSettings(httplib::Response &res);
    void _HandleSettings(const httplib::Request &req, httplib::Response &res);
    void _HandleRecord(const httplib::Request &req, httplib::Response &res);
    void _HandleClock(const httplib::Request &req, httplib::Response &res);

    // OutMux::IClient
    void Write(const uint8_t *buffer, size_t size) override;
};

} // namespace Native
