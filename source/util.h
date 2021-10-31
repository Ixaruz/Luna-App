#pragma once
#include <string>
#include <cstring>
#include <cstdio>
#include <cstdarg>		//va_list and stuff
#include <cstdlib>
#include "helpers\dmnt\dmntcht.h"
#include "helpers\debugger\debugger.hpp"
#include <switch.h>
#include <thread>
#include <chrono>
#include "luna.h"

#ifndef UTIL_H
#define UTIL_H

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
extern const u64 PlayerOtherOffset;
extern const u64 ItemCollectBitOffset;
extern const u64 StorageSizeOffset;
extern const u64 Pocket1SizeOffset;
extern const u64 ExpandBaggageOffset;

extern const u64 houseSize;
extern const u64 houseLvlOffset;
extern const u64 EventFlagOffset;

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

    std::string GetLastTimeSaved(u64 mainAddr);

    bool isServiceRunning(const char* serviceName);
}
#endif // UTIL_H
