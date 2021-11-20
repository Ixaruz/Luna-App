#pragma once
#include <string>
#include <cstring>
#include <vector>
#include <cstdio>
#include <cstdarg>		//va_list and stuff
#include <map>
#include <cstdlib>
#include <sys/stat.h>
#include "helpers\dmnt\dmntcht.h"
#include "helpers\debugger\debugger.hpp"
#include <switch.h>
#include <thread>
#include <chrono>
#include "luna.h"

#ifndef UTIL_H
#define UTIL_H

#define MHz *1E6

extern const s64 SaveHeaderSize;

extern const u64 mainSize;
extern const u64 GSavePlayerVillagerAccountOffset;
extern const u64 GSavePlayerVillagerAccountSize;
extern const u64 GAccountTableOffset;
extern const u64 DreamIDOffset;
extern const u64 DreamInfoSize;

extern const u64 playerSize;
extern const u64 playersOffset;

//taken from NHSE
extern const u64 PersonalID;
extern const u64 EventFlagsPlayerOffset;
extern const u64 ItemCollectBitOffset;
extern const u64 ItemRemakeCollectBitOffset;
extern const u64 RecipesOffset;
extern const u64 StorageSizeOffset;
extern const u64 Pocket1SizeOffset;
extern const u64 ExpandBaggageOffset;

extern const u64 houseSize;
extern const u64 houseLvlOffset;
extern const u64 EventFlagOffset;
extern const u64 SaveFgOffset;
extern const u64 SpecialityFruitOffset;

extern const u32 REV_200_MAIN_SIZE;
extern const u32 REV_200_PERSONAL_SIZE;

extern const std::vector<u16> MainmenuRecipes;

extern const std::vector<u16> PrettyGoodToolsRecipesRecipes;

extern const std::vector<u16> BeAChefRecipes;

extern const std::map<u16, u16> TownfruitSmoothiesMap;

extern const std::vector<u64> BID;

class FileHashRegion
{
public:
    u32 HashOffset;
    u32 Size;
    u32 getBeginOffset() const { return HashOffset + 4; }
    u32 getEndOffset() const { return getBeginOffset() + Size; }

    FileHashRegion(u32 hashOfs, u32 size) : HashOffset(hashOfs), Size(size)
    {}
};

extern const std::vector<FileHashRegion*> REV_200_MAIN;
extern const std::vector<FileHashRegion*> REV_200_PERSONAL;

struct IslandName {
    u16 name[0xB];
};

namespace util
{

    //removes char from C++ string
    void stripChar(char _c, std::string& _s);

    std::string getIslandNameASCII(u64 playerAddr);

    std::string getDreamAddrString(u64 mainAddr);

    TimeCalendarTime getDreamTime(u64 mainAddr);

    IslandName getIslandName(u64 playerAddr);

    u64 FollowPointerMain(u64 pointer, ...);

    bool getFlag(unsigned char data[], int bitIndex);

    void setBitBequalsA(u16 arrA[], int arrlen, unsigned char* B, int bitIndexOffset);

    void setBitBequalsA(u16 A, unsigned char* B, int bitIndexOffset);

    void SetFlag(u8* Data, int bitIndex, u16 value);

    std::string GetLastTimeSaved(u64 mainAddr);

    bool isServiceRunning(const char* serviceName);

    void overclockSystem(bool enable);
}
#endif // UTIL_H
