#include "../Common/UdpWorker.h"
#include <iostream>

class CUdpServer : public CUdpWorkerBase
{
public:
    CUdpServer();

    virtual void Init(const std::string& addr, int port) override;

    virtual void Run() override;
};