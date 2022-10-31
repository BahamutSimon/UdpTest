#pragma once
#include <stdint.h>
#include <stddef.h>
#include <string>
#include <vector>

class CUtils
{
public:
    static uint32_t Crc32(uint32_t crc, const unsigned char* buf, size_t len);
    static int64_t GetCurTimeMilliseconds();
    static uint64_t GetRandom();
    static bool ParseCommandLine(int argc, char* argv[], std::string& outAddress, int& outPort, int &outDropChance, std::vector<std::string>& outFiles);
};