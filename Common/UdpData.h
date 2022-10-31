#pragma once

#include <stdint.h>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <random>

#define PACKET_HEADER_SIZE 17// 4(uint32_t) + 4(uint32_t) + 1(uint8_t) + 8(id)
#define PACKET_DATA_MAX_SIZE 1455////1472 - header
#define PACKET_RESEND_TIME_MS 1000//1 sec for client to resend PUT packet 

//Single packet
struct SUdpPacketPutInfo
{
    size_t m_dataSize = 0;
    int64_t m_lastSendTime = -1;
};

enum EPacketType : uint8_t
{
    EPT_ACK = 0,
    EPT_PUT = 1,
};

#pragma pack(push, 1)
struct SUdpPacket
{
public:
    SUdpPacket();
    SUdpPacket(uint64_t id, uint32_t seqNumber, uint32_t seqTotal, const std::vector <unsigned char>& datablock, int dataIndex, int dataSize);
    SUdpPacket(const SUdpPacket& packet, int dataSize, uint8_t type);
    unsigned char* GetData();

    uint32_t m_seqNumber;
    uint32_t m_seqTotal;
    uint8_t m_type;
    uint64_t m_id;
    unsigned char m_data[PACKET_DATA_MAX_SIZE];
};

#pragma pack(pop)


//Single file splitted into packets
class CFileData
{
public:
    CFileData();
    CFileData(uint64_t id);
    bool CreateFromFileData(std::vector <unsigned char>& fileMemblock);
    SUdpPacket* GetPacketToSend(size_t& outSize);
    void OnPacketReceived(SUdpPacket* udpPacket, int size, bool& outComplete);
    uint32_t GetCrc() const;
    size_t GetPacketsCount() const;

private:
    void InitDataPacketsFromMemblock();
    void InitMemblockFromDataPackets();
    void Init();

private:
    uint64_t m_id = 0;
    uint32_t m_crc = 0;
    std::vector <unsigned char> m_memblock;
    std::vector<std::unique_ptr<SUdpPacket>> m_packets;
    std::vector<SUdpPacketPutInfo> m_packetsMeta;
    std::vector<int> m_packetsToSend;
    std::unordered_set<uint32_t> m_packetsReceived;
    bool m_bServerCrcReceived = false;

    //for packets shuffling
    std::random_device m_rd;
    std::default_random_engine m_dre;
};

//Storage for files
class CFileStorage
{
public:
    CFileStorage();
    CFileData* CreateFileData(const std::string& fileName);
    CFileData* CreateFileDataById(uint64_t id);
    CFileData* GetFileData(uint64_t id);
    void OnPacketReceived(SUdpPacket* udpPacket, int size, bool& outComplete);
    SUdpPacket* GetDataToSend(size_t& outSize);

private:
    typedef std::unordered_map<uint64_t, std::unique_ptr<CFileData>> TFileDataMap;
private:
    TFileDataMap m_fileDataMap;

};