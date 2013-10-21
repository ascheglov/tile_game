#pragma warning(disable: 4913)
#define _WIN32_WINNT 0x0601
#include "Server.hpp"

#define CATCH_CONFIG_RUNNER
#include "catch_wrap.hpp"

int main(int argc, char* argv[])
{
    bool runServer = false;
    if (argc > 1 && argv[argc - 1] == std::string("--run-server"))
    {
        runServer = true;
        --argc;
        argv[argc] = nullptr;
    }

    auto result = Catch::Session().run(argc, argv);

    if (!runServer)
        return result;

    Server srv;
    srv.start();
    std::cout << "Server started.\n";
    srv.run();
}