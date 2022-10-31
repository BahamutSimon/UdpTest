#include "UdpData.h"
#include <cstdio>
#include <string.h>
#include <iostream>
#include <cstring>
#include <fstream>
#include "Utils.h"

#include<iostream>
///////
//SUdpPacket
///////

SUdpPacket::SUdpPacket()
{
    m_seqNumber = 0;
    m_seqTotal = 0;
    m_type = EPacketType::EPT_PUT;
    m_id = 0;
    //memset(m_id, 0x0, sizeof(m_id));
}

SUdpPacket::SUdpPacket(uint64_t id, uint32_t seqNumber, uint32_t seqTotal, const std::vector <unsigned char>& datablock, int dataIndex, int dataSize)
{
    m_seqNumber = seqNumber;
    m_seqTotal = seqTotal;
    m_type = EPacketType::EPT_PUT;
    m_id = id;
    std::memcpy(&m_data[0], (void*)&datablock[dataIndex], dataSize);

}

SUdpPacket::SUdpPacket(const SUdpPacket& packet, int dataSize, uint8_t type,  size_t receivedCount)
{
    m_seqNumber = packet.m_seqNumber;
    m_seqTotal = receivedCount;
    m_type = type;
    m_id = packet.m_id;
    std::memcpy(&m_data[0], (void*)&packet.m_data[0], dataSize);

}

unsigned char* SUdpPacket::GetData()
{
    return &m_data[0];
}

///////
//CFileData
///////

CFileData::CFileData()
{
    Init();
}

CFileData::CFileData(uint64_t id)
{
    m_id = id;
    Init();
}

void CFileData::Init()
{
    m_dre = std::default_random_engine{m_rd()};
}

 bool CFileData::CreateFromFileData(std::vector <unsigned char>& fileMemblock)
 {
    std::swap(fileMemblock, m_memblock);
    m_crc = CUtils::Crc32(0, &m_memblock[0], m_memblock.size());
    InitDataPacketsFromMemblock();
    //InitMemblockFromDataPackets();
    return true;
 }

uint32_t CFileData::GetCrc() const
{
    return m_crc;
}

size_t CFileData::GetPacketsCount() const
{
    return m_packets.size();
}

 void CFileData::InitDataPacketsFromMemblock()
 {
    if (m_memblock.empty())
        return;

    uint32_t packetsCount = (m_memblock.size() - 1) / PACKET_DATA_MAX_SIZE + 1;
    m_packets.resize(packetsCount);
    m_packetsMeta.resize(packetsCount);
    int index = 0;
    for (int i = 0; i < packetsCount; ++i){
        int dataSize = i < packetsCount - 1 ? PACKET_DATA_MAX_SIZE : m_memblock.size() % PACKET_DATA_MAX_SIZE;
        //int dataSize = PACKET_DATA_MAX_SIZE;//i < packetsCount - 1 ? PACKET_DATA_MAX_SIZE : m_memblock.size() % PACKET_DATA_MAX_SIZE;
        m_packets[i] = std::make_unique<SUdpPacket>(m_id, i, packetsCount, m_memblock, PACKET_DATA_MAX_SIZE * i, dataSize);
        m_packetsMeta[i].m_dataSize = dataSize; 

        m_packetsToSend.push_back(i);
    }

    std::shuffle(std::begin(m_packetsToSend), std::end(m_packetsToSend), m_dre);
 }

 void CFileData::InitMemblockFromDataPackets()
 {
    if (m_packets.empty())
        return;

    m_memblock.clear();
    int newSize = (m_packets.size() - 1) * PACKET_DATA_MAX_SIZE + m_packetsMeta[m_packetsMeta.size() - 1].m_dataSize;

    m_memblock.resize(newSize);
    for (int i = 0; i < m_packets.size(); ++i){
        std::memcpy((void*)&m_memblock[i * PACKET_DATA_MAX_SIZE], m_packets[i]->GetData(), m_packetsMeta[i].m_dataSize);
        
    }
    /*std::string newName = std::to_string(m_id);
    std::ofstream wf(newName, std::ios::out | std::ios::binary);
    wf.write((char*)(&m_memblock[0]), m_memblock.size());
    wf.close();*/
    m_crc = CUtils::Crc32(0, &m_memblock[0], m_memblock.size());
    std::cout << "File combined. Crc: " << m_crc << std::endl;
 }


SUdpPacket* CFileData::GetPacketToSend(size_t& outSize)
{
    if (m_packetsToSend.empty())
        return nullptr;

    int indexToSend = -1;
    int packetsToSendIndex = -1;

    std::string SendLog = "Sending";
    int64_t curTime = CUtils::GetCurTimeMilliseconds();
    for (int i = m_packetsToSend.size() - 1; i >= 0; --i)
    {
        int index = m_packetsToSend[i];
        if (m_packetsMeta[index].m_lastSendTime < 0 || (curTime - m_packetsMeta[index].m_lastSendTime) >= PACKET_RESEND_TIME_MS){
            indexToSend = index;
            packetsToSendIndex = i;
            if (m_packetsMeta[index].m_lastSendTime > 0)
                SendLog = "Resending";
            break;
        }
    }
    if (indexToSend >= 0){
        SUdpPacket* toSend = m_packets[indexToSend].get();

        if (toSend->m_type == EPacketType::EPT_PUT){
            outSize = m_packetsMeta[indexToSend].m_dataSize;
            m_packetsMeta[indexToSend].m_lastSendTime = curTime;
        } else
        {
            outSize = 0;

            if (indexToSend < m_packetsToSend.size() - 1)
                std::iter_swap(m_packetsToSend.begin() + packetsToSendIndex, m_packetsToSend.begin() + m_packetsToSend.size() - 1);
            m_packetsToSend.pop_back();
            if (toSend->m_seqTotal == m_packets.size())
               outSize += sizeof(uint32_t); 
        }

        outSize += PACKET_HEADER_SIZE;
        std::cout << SendLog <<" id: " << toSend->m_id << " type: "  << (int)toSend->m_type << " num: " << toSend->m_seqNumber << " / " << toSend->m_seqTotal << std::endl;
        return toSend;
    }
    return nullptr;
    
}

void CFileData::OnPacketReceived(SUdpPacket* udpPacket, int size, bool& outComplete)
{
    outComplete = false;
    std::cout << "Received id: " << m_id << " type: "  << (int)udpPacket->m_type << " num: " << udpPacket->m_seqNumber << " / " << udpPacket->m_seqTotal << std::endl;
    uint32_t seqNumber = udpPacket->m_seqNumber;
    if (udpPacket->m_type == EPacketType::EPT_PUT)
    {
        uint32_t packetsCount = udpPacket->m_seqTotal;
        if (m_packets.empty()){
            m_packets.resize(packetsCount);
            m_packetsMeta.resize(packetsCount);
        }

        int dataSize = size - PACKET_HEADER_SIZE;
        m_packetsReceived.insert(seqNumber);
        m_packets[seqNumber] = std::make_unique<SUdpPacket>(*udpPacket, dataSize, EPacketType::EPT_ACK, m_packetsReceived.size());
        m_packetsMeta[seqNumber].m_dataSize = dataSize;
        m_packetsToSend.push_back(seqNumber);
        if (m_packetsReceived.size() == packetsCount){
            if (m_crc == 0)
                InitMemblockFromDataPackets();
            std::memcpy(m_packets[seqNumber]->m_data, &m_crc, sizeof(m_crc));
        }
    } 
    else 
    {
        for (int i = 0; i < m_packetsToSend.size(); ++i)
        {
            int index = m_packetsToSend[i];
            if (m_packets[index]->m_seqNumber != seqNumber)
                continue;

            if (i < m_packetsToSend.size() - 1)
                std::iter_swap(m_packetsToSend.begin() + i, m_packetsToSend.begin() + m_packetsToSend.size() - 1);
            m_packetsToSend.pop_back();
        }

        if (udpPacket->m_seqTotal == m_packets.size())
        {
            uint32_t serverCrc = 0;
            std::memcpy(&serverCrc, &udpPacket->m_data, sizeof(uint32_t));
            std::string crcLog = serverCrc == m_crc ? "Valid" : "ERROR";
            std::cout << "Received crc from server: " << serverCrc <<" Local crc: "  << m_crc << " " << crcLog <<std::endl;
            outComplete = true;
        }
        
    }
}

//////
//CFileStorage
//////

CFileStorage::CFileStorage()
{
}

CFileData* CFileStorage::CreateFileData(const std::string& fileName)
{
    std::ifstream input (fileName, std::ios::in|std::ios::binary|std::ios::ate);

    if (!input.is_open())
    {
        input.close();
        std::cout << "Error openning file: " << fileName << std::endl;
        return nullptr;
    }

    std::vector <unsigned char> memblock;
    std::streampos size = input.tellg();
    memblock.resize(size);
    input.seekg (0, std::ios::beg);
    input.read ((char*)(&memblock[0]), size);
    input.close();

    CFileData* data = CreateFileDataById(CUtils::GetRandom());
    data->CreateFromFileData(memblock);
    std::cout << "File: " << fileName << " Splitted into " << data->GetPacketsCount() << " packets " << (int)size << " Crc: " << data->GetCrc() << std::endl;
    return data;
}

CFileData* CFileStorage::CreateFileDataById(uint64_t id)
{
    m_fileDataMap[id] = std::make_unique<CFileData>(id);
    return m_fileDataMap[id].get();
}

CFileData* CFileStorage::GetFileData(uint64_t id)
{
    TFileDataMap::iterator it = m_fileDataMap.find(id);
    if (it == m_fileDataMap.end()) 
        return nullptr;
    
    return it->second.get();
}

void CFileStorage::OnPacketReceived(SUdpPacket* udpPacket, int size, bool& outComplete)
{
    uint64_t id = udpPacket->m_id;
    CFileData* data = GetFileData(id);
    if (data == nullptr){
        data = CreateFileDataById(id);
    }
    data->OnPacketReceived(udpPacket, size, outComplete);
    
}

SUdpPacket* CFileStorage::GetDataToSend(size_t& outSize)
{
    for (auto& [_, fileData] : m_fileDataMap)
    {
        SUdpPacket* result =  fileData->GetPacketToSend(outSize);
        if (result != nullptr)
            return result;
    }

    return nullptr;
}
