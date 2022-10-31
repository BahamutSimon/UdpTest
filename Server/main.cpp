#include "UdpServer.h"
#include <string.h>
#include "../Common/Utils.h"

int main(int argc, char* argv[])
{
    std::string addr = "127.0.0.1";
    int port = 40000;
    int dropChance = 0;
    std::vector<std::string> files;
    if (!CUtils::ParseCommandLine(argc, argv, addr, port, dropChance, files))
    {
        std::cout << "Error parsing arguments" << std::endl;
        return 0;
    }

    std::cout << "Server started: Address: " << addr << " Port: " << port << " Drop chance: " << dropChance << std::endl;


    CUdpServer server;
    server.Init("127.0.0.1", 40000);
    server.SetPacketDropChance(dropChance);
    server.Run();

    return 0;
}