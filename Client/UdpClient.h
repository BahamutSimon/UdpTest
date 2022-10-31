#pragma once
#include "../Common/UdpWorker.h"

class CUdpClient : public CUdpWorkerBase
{
public:
    CUdpClient();
    CUdpClient(std::vector<std::string>& files);
    virtual void Run() override;

private:
    std::vector<std::string> m_files;
};