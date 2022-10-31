#pragma once
#include <string>
#include <memory>
#include <chrono>
#include <thread>
#include "UdpData.h"

class CUdpSocketBase
{
public:
    CUdpSocketBase();
    ~CUdpSocketBase();

    virtual void Init(const std::string& addr, int port);
    int Send(void* msg, size_t size);
    int Receive(void*msg, size_t max_size);
    

protected:
    void Bind();

protected:
    int                 f_socket;
    int                 f_port;
    std::string         f_addr;
    struct addrinfo*    f_addrinfo;
};

class CUdpWorkerBase : public CUdpSocketBase
{
public:
    CUdpWorkerBase();
    void SetPacketDropChance(int dropChance);
    virtual void Run() = 0;
    virtual void OnPacketReceived (SUdpPacket* udpPacket, int size, bool& outComplete);

protected:
    std::unique_ptr<CFileStorage> m_fileStorage;
    int m_packetDropChance = 0;//in percents. 0 - no drop, 100 - drop all incoming data
};