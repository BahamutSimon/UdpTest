#include "UdpWorker.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdexcept>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include "iostream"

///////////
//CUdpSocketBase
///////////

CUdpSocketBase::CUdpSocketBase()
{
    f_port = 0;
}

CUdpSocketBase::~CUdpSocketBase()
{
    freeaddrinfo(f_addrinfo);
    close(f_socket);
}

void CUdpSocketBase::Init(const std::string& addr, int port)
{
    f_port = port;
    f_addr = addr;
    char decimal_port[16];
    snprintf(decimal_port, sizeof(decimal_port), "%d", f_port);
    decimal_port[sizeof(decimal_port) / sizeof(decimal_port[0]) - 1] = '\0';
    addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;
    int r(getaddrinfo(f_addr.c_str(), decimal_port, &hints, &f_addrinfo));
    if(r != 0 || f_addrinfo == NULL)
    {
        throw std::runtime_error(("invalid address or port: \"" + f_addr + ":" + decimal_port + "\"").c_str());
    }
    f_socket = socket(f_addrinfo->ai_family, SOCK_DGRAM | SOCK_CLOEXEC, IPPROTO_UDP);
    if(f_socket == -1)
    {
        freeaddrinfo(f_addrinfo);
        throw std::runtime_error(("could not create socket for: \"" + f_addr + ":" + decimal_port + "\"").c_str());
    }

    int flags = fcntl(f_socket, F_GETFL, 0);
    fcntl(f_socket, F_SETFL, flags | O_NONBLOCK);
}

void CUdpSocketBase::Bind()
{
    char decimal_port[16];
    snprintf(decimal_port, sizeof(decimal_port), "%d", f_port);
    int r = bind(f_socket, f_addrinfo->ai_addr, f_addrinfo->ai_addrlen);
    if(r != 0)
    {
        freeaddrinfo(f_addrinfo);
        close(f_socket);
        throw std::runtime_error(("could not bind UDP socket with: \"" + f_addr + ":" + decimal_port + "\"").c_str());
    }
}

int CUdpSocketBase::Send(void* msg, size_t size)
{
    return sendto(f_socket, msg, size, 0, f_addrinfo->ai_addr, f_addrinfo->ai_addrlen);
}

int CUdpSocketBase::Receive(void* msg, size_t max_size)
{
    return recvfrom(f_socket, msg, max_size, 0, f_addrinfo->ai_addr, &f_addrinfo->ai_addrlen);
}
////////
//CUdpWorkerBase
////////

CUdpWorkerBase::CUdpWorkerBase()
{
    m_fileStorage = std::make_unique<CFileStorage>();
     std::srand(std::time(nullptr));
}

void CUdpWorkerBase::SetPacketDropChance(int dropChance)
{
    m_packetDropChance = dropChance;
}

void CUdpWorkerBase::OnPacketReceived (SUdpPacket* udpPacket, int size, bool& outComplete)
{
    if (m_packetDropChance > 0 && (std::rand() % 101 <= m_packetDropChance))
    {
        std::cout << "Packet dropped id: " << udpPacket->m_id << " type: "  << (int)udpPacket->m_type << " num: " << udpPacket->m_seqNumber << " / " << udpPacket->m_seqTotal << std::endl;
        return;
    }
    m_fileStorage->OnPacketReceived(udpPacket, size, outComplete);
}
