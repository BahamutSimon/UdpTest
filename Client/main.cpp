#include "UdpClient.h"
#include "../Common/Utils.h"
#include <iostream>


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

    if (files.empty())
    {
        std::cout << "No files provided" << std::endl;
        return 0;
    }

    std::cout << "Client started: Address: " << addr << " Port: " << port << " Drop chance: " << dropChance << " Files:";
    for (auto& file : files)
    {
        std::cout << " " << file;
    }
    std::cout << std::endl;

    CUdpClient client(files);
    client.Init(addr, port);
    client.SetPacketDropChance(dropChance);
    client.Run();
    return 0;
}