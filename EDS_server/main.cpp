#include "App/cAppCore.h"
#include <iostream>

int main()
{
    Sys::cAppCore oApp;

    const unsigned short uWsPort = 9000;
    const unsigned short uHttpPort = 8080;

    if (!oApp.fnInit(uWsPort, uHttpPort)) {
        std::cerr << "Failed to init server\n";
        return 1;
    }

    std::cout << "Server running.\n";
    std::cout << "WS:   ws://127.0.0.1:" << uWsPort << "\n";
    std::cout << "HTTP: http://127.0.0.1:" << uHttpPort << "/health\n";

    oApp.fnRun();
    return 0;
}
