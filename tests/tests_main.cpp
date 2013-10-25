#pragma warning(disable: 4913)
#define _WIN32_WINNT 0x0601
#include "server/Menu.hpp"
#include "server/Server.hpp"

#define CATCH_CONFIG_RUNNER
#include "catch_wrap.hpp"

static void serverMain()
{
    auto worldMap =
        "........" // 0
        ".?WWWW.."
        "..?..W.."
        "..W?...."
        "...W?..." // 4
        ".....?.."
        "......?."
        "........"
        ;

    GameCfg cfg;
    Server srv{cfg, worldMap};
    srv.start("127.0.0.1", 4080, std::cout);
    std::cout << "Press [q] to quit or [h] for help\n";
    
    Menu menu;
    for (;;)
    {
        srv.tick();
        menu.tick();
        if (menu.quitRequested())
            break;

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    std::cout << "stopping...\n";
    srv.stop();
}

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

    serverMain();
}