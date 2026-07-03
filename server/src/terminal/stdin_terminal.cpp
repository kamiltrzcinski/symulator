// server/src/terminal/stdin_terminal.cpp

#include "server/terminal/stdin_terminal.hpp"

#include <iostream>
#include <string>

#ifndef _WIN32
#include <poll.h>
#include <unistd.h>
#endif

namespace server::terminal
{

namespace
{

// Returns true when a full line is ready on stdin.  On POSIX, polls with a
// timeout so the reader thread can observe stop_token requests; on Windows
// there is no portable non-blocking console read, so the loop falls back to a
// blocking getline and only re-checks the token after each entered line.
bool wait_for_input(const std::stop_token& stoken)
{
#ifndef _WIN32
    pollfd fd{.fd = STDIN_FILENO, .events = POLLIN, .revents = 0};
    while (!stoken.stop_requested())
    {
        const int ready = ::poll(&fd, 1, /*timeout_ms=*/200);
        if (ready > 0)
            return true;
        if (ready < 0)
            return false;  // poll error — give up reading
    }
    return false;
#else
    (void)stoken;
    return true;
#endif
}

}  // namespace

StdinTerminal::StdinTerminal(TerminalSession& session) : session_(session) {}

StdinTerminal::~StdinTerminal()
{
    stop();
}

void StdinTerminal::start()
{
    if (thread_.joinable())
        return;  // already running

    thread_ = std::jthread([this](std::stop_token st) { run(std::move(st)); });
}

void StdinTerminal::stop()
{
    if (!thread_.joinable())
        return;
    thread_.request_stop();
    thread_.join();
}

void StdinTerminal::run(std::stop_token stoken)
{
    std::cout << "[terminal] built-in terminal ready — type `help`\n> " << std::flush;

    std::string line;
    while (!stoken.stop_requested())
    {
        if (!wait_for_input(stoken))
            break;

        if (!std::getline(std::cin, line))
            break;  // EOF — stdin closed

        const std::string output = session_.process_line(line);
        if (!output.empty())
            std::cout << output << "\n";
        std::cout << "> " << std::flush;
    }
}

}  // namespace server::terminal
