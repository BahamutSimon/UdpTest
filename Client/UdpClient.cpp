#include "UdpClient.h"
#include <iostream>
#include <memory>

CUdpClient::CUdpClient()
{
    
}

CUdpClient::CUdpClient(std::vector<std::string>& files)
: m_files(files)
{
}

void CUdpClient::Run()
{
    SUdpPacket packet;
    int filesCount = 0;
    for (auto& file : m_files)
    {
        if (m_fileStorage->CreateFileData(file) != nullptr)
            filesCount++;
    }
    if (filesCount <= 0)
    {
        std::cout << "No files provided" << std::endl;
        return;
    }

    while (true)
    {
        bool bComplete = false;
        int res = Receive(&packet, (sizeof(packet)));
        if (res > 0){
            OnPacketReceived(&packet, res, bComplete);
            if (bComplete)
                filesCount--;
        }

        if (filesCount <= 0)
            break;

        size_t packetSize = 0;
        SUdpPacket* packetToSend = m_fileStorage->GetDataToSend(packetSize);
        if (packetToSend != nullptr)
            int res = Send(packetToSend, packetSize);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

    }
}