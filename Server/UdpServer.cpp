#include "UdpServer.h"
#include "../Common/Utils.h"

CUdpServer::CUdpServer()
{

}

void CUdpServer::Init(const std::string& addr, int port)
{
    CUdpWorkerBase::Init(addr, port);
    Bind();
}

void CUdpServer::Run()
{
    SUdpPacket packet;
    bool waitingData = false;
    while(true)
    {
        int res = Receive(&packet, (sizeof(packet)));
        if (res > 0){
            OnPacketReceived(&packet, res, waitingData);
        }

        size_t packetSize = 0;
        SUdpPacket* packetToSend = m_fileStorage->GetDataToSend(packetSize);
        if (packetToSend != nullptr)
            int res = Send(packetToSend, packetSize);

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

}