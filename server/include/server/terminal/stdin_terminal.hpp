// server/include/server/terminal/stdin_terminal.hpp
//
// stdin/stdout adapter for the built-in server terminal.  All session logic
// (login, permissions, dispatch) lives in TerminalSession; this class only
// moves lines in and text out.  Future transports (telnet, admin websocket)
// implement ITerminalTransport against the same session.

#pragma once

#include "server/terminal/terminal_session.hpp"

#include <thread>

namespace server::terminal
{

class ITerminalTransport
{
public:
    virtual ~ITerminalTransport() = default;
    virtual void start() = 0;
    virtual void stop() = 0;
};

class StdinTerminal final : public ITerminalTransport
{
public:
    /// @param session  Must outlive this object.
    explicit StdinTerminal(TerminalSession& session);
    ~StdinTerminal() override;

    void start() override;
    void stop() override;

private:
    void run(std::stop_token stoken);

    TerminalSession& session_;
    std::jthread thread_;
};

}  // namespace server::terminal
