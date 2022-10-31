#include "Utils.h"
#include <chrono>
#include <random>
#include <limits>

uint32_t CUtils::Crc32(uint32_t crc, const unsigned char* buf, size_t len)
{
    int k;
    crc = ~crc;
    while (len--){
        crc ^= *buf++;
        for (k = 0; k < 8; k++)
            crc = crc & 1 ? (crc >> 1) ^ 0x82f63b78 : crc >> 1;
    }
    return ~crc;
}

int64_t CUtils::GetCurTimeMilliseconds()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();;
}

int64_t CUtils::GetRandom()
{
    std::random_device rd;
    std::mt19937_64 eng(rd());
    std::uniform_int_distribution<unsigned long long> distr;
    return distr(eng);
}

bool CUtils::ParseCommandLine(int argc, char* argv[], std::string& outAddress, int& outPort, int &outDropChance, std::vector<std::string>& outFiles)
{
        std::vector<std::string> args(argv + 1, argc + argv);

        for (int i = 0; i < args.size();)
        {
            //address
            if (args[i] == "-a")
            {
                i++;
                if (i >= args.size())
                    return true;

                outAddress = args[i];
                i++;
            }
            //port
            else if(args[i] == "-p")
            {
                i++;
                if (i >= args.size())
                    return true;

                try
                {
                    outPort = std::stoi(args[i]);
                }
                catch(const std::exception& e)
                {
                    return false;
                }
                i++;
                
            }
            //drop chance
            else if(args[i] == "-d")
            {
                i++;
                if (i >= args.size())
                    return true;

                try
                {
                    outDropChance = std::stoi(args[i]);
                }
                catch(const std::exception& e)
                {
                    return false;
                }
                i++;
                
            }
            //files
            else if(args[i] == "-f")
            {
                i++;
                if (i >= args.size())
                    return true;

                outFiles.push_back(args[i]);
                i++;
            } else
             {
                i++;
            }
        }

        return true;
}