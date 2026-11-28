#include "Network.hpp"
#include "../app/Settings.hpp"
#include "../app/Shell.hpp"
#include "../app/Buzzer.hpp"
#include "WebUI.hpp"

#include <httplib.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <chrono>
#include <cinttypes>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0 // macOS: SIGPIPE is suppressed via SO_NOSIGPIPE below
#endif

// Unprivileged ports by default (23 / 80 need root on Linux/macOS).
#ifndef NATIVE_HTTP_PORT
#define NATIVE_HTTP_PORT 8080
#endif
#ifndef NATIVE_SHELL_PORT
#define NATIVE_SHELL_PORT 2323
#endif

namespace Native {

namespace {

uint32_t NowMs()
{
    using namespace std::chrono;
    return static_cast<uint32_t>(
        duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count());
}

int MakeListenSocket(uint16_t port)
{
    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        return -1;
    }
    int one = 1;
    ::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);
    if (::bind(fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0 ||
        ::listen(fd, 1) < 0) {
        ::close(fd);
        return -1;
    }
    return fd;
}

} // namespace

Network::Network(OutMux &outMux, Settings &settings, Shell &shell, Buzzer &buzzer)
    : _outMux{outMux}
    , _settings{settings}
    , _shell{shell}
    , _buzzer{buzzer}
{
}

Network::~Network()
{
    if (_running.load()) {
        SwitchOff();
    }
}

void Network::Setup()
{
    _webServer = std::make_unique<httplib::Server>();
    auto &srv = *_webServer;

    for (const auto &endpoint : WebUI::STRING_ENDPOINTS) {
        const std::string route = endpoint.route;
        const std::string type = endpoint.contentType;
        const std::string body = std::string{endpoint.content};
        srv.Get(route, [type, body](const httplib::Request &, httplib::Response &res) {
            res.set_content(body, type);
        });
    }

    for (const auto &endpoint : WebUI::BINARY_ENDPOINTS) {
        const std::string route = endpoint.route;
        const std::string type = endpoint.contentType;
        const std::string body = std::string{reinterpret_cast<const char *>(endpoint.content), static_cast<size_t>(endpoint.contentLength)};
        srv.Get(route, [type, body](const httplib::Request &, httplib::Response &res) {
            res.set_content(body, type);
        });
    }

    // SPA fallback, like onNotFound() on the ESP32.
    srv.set_error_handler([](const httplib::Request &, httplib::Response &res) {
        if (res.status == 404) {
            res.status = 200;
            res.set_content(WebUI::index_html, "text/html");
        }
    });

    srv.Get("/settings", [this](const httplib::Request &req, httplib::Response &res) {
        _HandleSettings(req, res);
    });
    srv.Post("/settings", [this](const httplib::Request &req, httplib::Response &res) {
        _HandleSettings(req, res);
    });
    srv.Get("/record", [this](const httplib::Request &req, httplib::Response &res) {
        _HandleRecord(req, res);
    });
    srv.Get("/clock", [this](const httplib::Request &req, httplib::Response &res) {
        _HandleClock(req, res);
    });
    srv.Post("/clock", [this](const httplib::Request &req, httplib::Response &res) {
        _HandleClock(req, res);
    });

    _lastActivityTimeMs.store(NowMs());
}

void Network::SwitchOn()
{
    if (_running.exchange(true)) {
        return;
    }
    _lastActivityTimeMs.store(NowMs());

    // bind_to_port first so we can report failure synchronously.
    if (!_webServer->bind_to_port("0.0.0.0", NATIVE_HTTP_PORT)) {
        std::fprintf(stderr, "Network: cannot bind HTTP port %d\n", NATIVE_HTTP_PORT);
        _running = false;
        return;
    }
    _httpThread = std::thread([this] { _webServer->listen_after_bind(); });
    _shellThread = std::thread([this] { _ShellTask(); });

    std::printf("Network: web UI on http://localhost:%d, shell on tcp port %d\n",
                NATIVE_HTTP_PORT, NATIVE_SHELL_PORT);
}

void Network::SwitchOff()
{
    if (!_running.exchange(false)) {
        return;
    }

    _outMux.SetClient(nullptr);
    _webServer->stop();
    if (_httpThread.joinable()) {
        _httpThread.join();
    }
    if (_shellThread.joinable()) {
        _shellThread.join(); // polls _running every 100 ms
    }
}

unsigned Network::GetInactivitySeconds()
{
    return (NowMs() - _lastActivityTimeMs.load()) / 1000;
}

void Network::_ShellTask()
{
    const int listenFd = MakeListenSocket(NATIVE_SHELL_PORT);
    if (listenFd < 0) {
        std::fprintf(stderr, "Network: cannot bind shell port %d\n", NATIVE_SHELL_PORT);
        return;
    }

    int clientFd = -1;

    auto dropClient = [&] {
        _outMux.SetClient(nullptr);
        std::lock_guard<std::mutex> lk{_writeMutex};
        if (clientFd >= 0) {
            ::close(clientFd);
        }
        clientFd = -1;
        _clientFd.store(-1);
    };

    while (_running.load()) {
        pollfd fds[2];
        nfds_t nfds = 0;
        // Like the ESP32 version: only accept when no client is attached.
        if (clientFd < 0) {
            fds[nfds++] = {listenFd, POLLIN, 0};
        } else {
            fds[nfds++] = {clientFd, POLLIN, 0};
        }

        int rc = ::poll(fds, nfds, 100);
        if (rc <= 0) {
            continue;
        }

        if (clientFd < 0) {
            if (fds[0].revents & POLLIN) {
                int fd = ::accept(listenFd, nullptr, nullptr);
                if (fd >= 0) {
                    int one = 1;
                    ::setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));
#ifdef SO_NOSIGPIPE
                    ::setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, &one, sizeof(one));
#endif
                    clientFd = fd;
                    _clientFd.store(fd);
                    _outMux.SetClient(this);
                }
            }
        } else if (fds[0].revents & (POLLIN | POLLHUP | POLLERR)) {
            uint8_t buf[32];
            ssize_t n = ::recv(clientFd, buf, sizeof(buf), 0);
            if (n > 0) {
                {
                    std::lock_guard<std::mutex> lk{_mutex};
                    _shell.ProcessInput(buf, static_cast<size_t>(n));
                }
                _lastActivityTimeMs.store(NowMs());
            } else {
                dropClient(); // 0 = orderly close, <0 = error
            }
        }
    }

    dropClient();
    ::close(listenFd);
}

void Network::_HandleGetSettings(httplib::Response &res)
{
    std::string response;
    auto addSetting = [&response](const char *key, const std::string &val) {
        if (!response.empty()) {
            response += '\n';
        }
        response += key;
        response += '=';
        response += val;
    };

    addSetting("id", std::to_string(static_cast<unsigned>(_settings.GetId())));

    const auto &key = _settings.GetKey();
    char buf[Settings::KEY_SIZE * 2 + 1];
    for (int i = 0; i < Settings::KEY_SIZE; ++i) {
        std::snprintf(buf + i * 2, 3, "%02X", static_cast<unsigned>(key[i]));
    }
    buf[Settings::KEY_SIZE * 2] = 0;
    addSetting("key", buf);

    addSetting("t-act", std::to_string(_settings.GetActiveMinutes()));
    addSetting("t-eco", std::to_string(_settings.GetEcoMinutes()));

    res.set_content(response, "text/plain");
}

void Network::_HandleSettings(const httplib::Request &req, httplib::Response &res)
{
    _lastActivityTimeMs.store(NowMs());
    std::lock_guard<std::mutex> lk{_mutex};

    if (req.method == "POST") {
        _shell.SetId(req.get_param_value("id").c_str());
        _shell.SetKey(req.get_param_value("key").c_str());
        _shell.SetActive(req.get_param_value("t-act").c_str());
        _shell.SetEco(req.get_param_value("t-eco").c_str());
        res.set_content(WebUI::index_html, "text/html");
    } else if (req.method == "GET") {
        _HandleGetSettings(res);
    } else {
        res.status = 405;
    }
}

void Network::_HandleRecord(const httplib::Request &req, httplib::Response &res)
{
    _lastActivityTimeMs.store(NowMs());

    if (req.method != "GET") {
        res.status = 405;
        return;
    }

    struct Collector : AOP::Recorder::IVisitor
    {
        std::string buffer;

        void OnCard(uint16_t card, uint32_t timestamp, void *) override
        {
            if (!buffer.empty()) {
                buffer += ' ';
            }
            unsigned s = timestamp % 60;
            timestamp /= 60;
            unsigned m = timestamp % 60;
            timestamp /= 60;
            char sts[64];
            std::snprintf(sts, sizeof(sts), "%" PRIu16 "-%u:%02u:%02u",
                          card, static_cast<unsigned>(timestamp), m, s);
            buffer += sts;
        }
    } collector;

    {
        std::lock_guard<std::mutex> lk{_mutex};
        _settings.GetRecorder().List(collector, &collector);
    }
    res.set_content(collector.buffer, "text/plain");
}

void Network::_HandleClock(const httplib::Request &req, httplib::Response &res)
{
    _lastActivityTimeMs.store(NowMs());
    std::lock_guard<std::mutex> lk{_mutex};

    if (req.method == "GET") {
        res.set_content(std::to_string(_settings.GetDateTime().ToUnix()), "text/plain");
    } else if (req.method == "POST") {
        if (req.body.empty()) {
            res.status = 400;
            res.set_content("No data", "text/plain");
            return;
        }
        const auto datetime = static_cast<uint32_t>(std::strtoul(req.body.c_str(), nullptr, 10));
        _settings.SetDateTime(datetime);
        res.set_content(req.body, "text/plain");
    } else {
        res.status = 405;
    }
}

// Called by OutMux from whichever thread produces shell output.
void Network::Write(const uint8_t *buffer, size_t size)
{
    std::lock_guard<std::mutex> lk{_writeMutex};
    const int fd = _clientFd.load();
    if (fd < 0) {
        return;
    }
    while (size > 0) {
        ssize_t n = ::send(fd, buffer, size, MSG_NOSIGNAL);
        if (n <= 0) {
            break; // the shell thread notices the dead socket on its next recv()
        }
        buffer += n;
        size -= static_cast<size_t>(n);
    }
}

} // namespace Native
