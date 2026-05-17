// server/src/main.cpp
// Entry point — single responsibility: start the server.

#include "server/session_server.hpp"

int main(int argc, char* argv[])
{
    server::SessionServer::from_args(argc, argv).run();
    return 0;
}
