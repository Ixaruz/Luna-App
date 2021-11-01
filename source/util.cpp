#include "util.h"

extern const s64 SaveHeaderSize = 0x110;

extern const u64 mainSize = 0x5279A0; //changed in 1.10.0 // GSaveLandOther - Header
extern const u64 GSavePlayerVillagerAccountOffset = 0x1E2290 - SaveHeaderSize; //changed in 1.10.0
extern const u64 GSavePlayerVillagerAccountSize = 0x48;
extern const u64 GAccountTableOffset = 0x10;
extern const u64 DreamIDOffset = 0x5264E8; //changed in 1.10.0
extern const u64 DreamInfoSize = 0x50;


extern const u64 playerSize = 0x36940;
extern const u64 playersOffset = 0x7A970; //changed in 1.9.0

//taken from NHSE
//*personal.dat*//
extern const u64 PersonalID = 0xAFA8;
extern const u64 EventFlagsPlayerOffset = 0xAFE0;
extern const u64 PlayerOtherOffset = 0x36a50 - SaveHeaderSize; //0x36940
extern const u64 ItemCollectBitOffset = 0xA058;
extern const u64 StorageSizeOffset = PlayerOtherOffset + SaveHeaderSize + 0x18C + (8 * 5000); //absolute file offset //changed in 1.7.0 0x4081C
extern const u64 Pocket1SizeOffset = PlayerOtherOffset + SaveHeaderSize + 0x10 + (8 * 20); //absolute file offset //changed in 1.7.0 0x36B00
extern const u64 ExpandBaggageOffset = 0x36BD8;

//*main.dat*//
extern const u64 houseSize = 0x26400;
extern const u64 houseLvlOffset = 0x308be4; //changed in 1.10.0
extern const u64 EventFlagOffset = 0x22d9b8; //changed in 1.10.0


/*hashes*/

extern const u32 REV_1110_MAIN_SIZE = 0x86D570;
extern const u32 REV_1110_PERSONAL_SIZE = 0x64160;

extern const std::vector<FileHashRegion*> REV_1110_MAIN = std::vector<FileHashRegion*> {
		new FileHashRegion(0x000110, 0x1e216c),
		new FileHashRegion(0x1e2280, 0x34582c),
		new FileHashRegion(0x527bc0, 0x03693c),
		new FileHashRegion(0x55e500, 0x02d70c),
		new FileHashRegion(0x58bd20, 0x03693c),
		new FileHashRegion(0x5c2660, 0x02d70c),
		new FileHashRegion(0x5efe80, 0x03693c),
		new FileHashRegion(0x6267c0, 0x02d70c),
		new FileHashRegion(0x653fe0, 0x03693c),
		new FileHashRegion(0x68a920, 0x02d70c),
		new FileHashRegion(0x6b8140, 0x03693c),
		new FileHashRegion(0x6eea80, 0x02d70c),
		new FileHashRegion(0x71c2a0, 0x03693c),
		new FileHashRegion(0x752be0, 0x02d70c),
		new FileHashRegion(0x780400, 0x03693c),
		new FileHashRegion(0x7b6d40, 0x02d70c),
		new FileHashRegion(0x7e4560, 0x03693c),
		new FileHashRegion(0x81aea0, 0x02d70c),
		new FileHashRegion(0x8485b0, 0x024fbc),
};

extern const std::vector<FileHashRegion*> REV_1110_PERSONAL = std::vector<FileHashRegion*> {
		new FileHashRegion(0x00110, 0x3693c),
		new FileHashRegion(0x36a50, 0x2d70c),
};




static const char verboten[] = { ',', '/', '\\', '<', '>', ':', '"', '|', '?', '*', '™', '©', '®' };


static bool isVerboten(const u16& t)
{
	for (unsigned i = 0; i < 13; i++)
	{
		if (t == verboten[i])
			return true;
	}

	return false;
}

static inline bool isASCII(const u16& t)
{
	return (t > 31 && t < 127);
}


std::string util::getIslandNameASCII(u64 playerAddr)
{
	//0x16 byte = 0xB wide-chars/uint_16
	u16 name[0xB] = { 0 };
	u16 namechar;
	u8 lastchar = 0;

	for (u8 i = 0; i < 0xB; i++) {
		dmntchtReadCheatProcessMemory(playerAddr + PersonalID + 0x4 + (i * 0x2), &namechar, 0x2);
		//make sure we can use this fuck string in a path
		if (isASCII(namechar) && !isVerboten(namechar)) {
			name[i] = namechar;
			lastchar = i;
		}
		else {
			name[i] = 0x0000;
		}
	}
	//make sure there is no space on path ends
	if (name[lastchar] == 0x20) {
		name[lastchar] = 0x0000;
	}
	//nullterminator pain
	u8 name_string[0x16] = { 0 };
	//pain
	utf16_to_utf8(name_string, name, sizeof(name_string) / sizeof(u8));

	return std::string((char*)name_string);
}


std::string util::getDreamAddrString(u64 mainAddr)
{
	u64 cDreamID = 0x0;
	char buffer[0x10] = { 0 };


	dmntchtReadCheatProcessMemory(mainAddr + DreamIDOffset, &cDreamID, sizeof(u64));
	sprintf(buffer, "%012li", cDreamID);

	std::string str1 = std::string(buffer).substr(0, 4);
	std::string str2 = std::string(buffer).substr(4, 4);
	std::string str3 = std::string(buffer).substr(8, 4);

	return std::string(str1 + "-" + str2 + "-" + str3);
}

TimeCalendarTime util::getDreamTime(u64 mainAddr)
{
	TimeCalendarTime dreamtime;
	u64 DreamTimeOffs = DreamIDOffset + 0x40;
	dmntchtReadCheatProcessMemory(mainAddr + DreamTimeOffs, &dreamtime, sizeof(TimeCalendarTime));
	return dreamtime;
}


IslandName util::getIslandName(u64 playerAddr)
{
	//0x16 byte = 0xB wide-chars/uint_16
	u16 name[0xB];

	dmntchtReadCheatProcessMemory(playerAddr + PersonalID + 0x4, name, 0x16);
	IslandName ret;
	memcpy(ret.name, name, sizeof(name));

	return ret;
}

void util::stripChar(char _c, std::string& _s)
{
	size_t pos = 0;
	while ((pos = _s.find(_c)) != _s.npos)
		_s.erase(pos, 1);
}


/**
 * @brief Follow a variable pointer path from main (last arg has to be 0xFFFFFFFFFFFFFFFF)
 */
u64 util::FollowPointerMain(u64 pointer, ...)
{
	u64 offset;
	va_list pointers;
	va_start(pointers, pointer);

	DmntCheatProcessMetadata metadata;
	dmntchtGetCheatProcessMetadata(&metadata);

	size_t bufferSize = sizeof offset;
#if DEBUG
	Result rc = 0;
	if (R_FAILED(rc = dmntchtReadCheatProcessMemory(metadata.main_nso_extents.base + pointer, &offset, bufferSize))) {
		printf("Memory Read failed.\n");
	}
#else
	dmntchtReadCheatProcessMemory(metadata.main_nso_extents.base + pointer, &offset, bufferSize); // since the inital pointer will be a valid offset(we assume anyways...) do a read64 call to it and store in offset
#endif


	//return 0xFFFFFFFFFFFFFFFF;
	pointer = va_arg(pointers, u64); // go to next argument
	while (pointer != 0xFFFFFFFFFFFFFFFF) // the last arg needs to be -1 in order for the while loop to exit
	{
		dmntchtReadCheatProcessMemory(pointer + offset, &offset, bufferSize);
		//return 0xFFFFFFFFFFFFFFFF;
		pointer = va_arg(pointers, u64);
	}
	va_end(pointers);
	return offset;
}

bool util::getFlag(unsigned char data[], int bitIndex)
{
	unsigned char b = data[bitIndex >> 3];
	unsigned char mask = 1 << (bitIndex & 7);
	return (b & mask) != 0;
}

void util::setBitBequalsA(u16 arrA[], int arrlen, unsigned char* B, int bitIndexOffset) {
	for (int i = 0; i < arrlen; i++) {
		if ((arrA[i] == 1) != (util::getFlag(B, bitIndexOffset + i))) {
			B[(bitIndexOffset + i) >> 3] ^= (1 << ((i + bitIndexOffset) & 7));
		}
	}
}

void util::setBitBequalsA(u16 A, unsigned char* B, int bitIndexOffset) {
	if ((A == 1) != (util::getFlag(B, bitIndexOffset))) B[bitIndexOffset >> 3] ^= (1 << (bitIndexOffset & 7));
}

std::string util::GetLastTimeSaved(u64 mainAddr)
{
	const char* date_format = "%02d.%02d.%04d @ %02d-%02d";
	char ret[128];
	TimeCalendarTime time;
	dmntchtReadCheatProcessMemory(mainAddr + 0x504D20, &time, 0x8);
	sprintf(ret, date_format, time.day, time.month, time.year, time.hour, time.minute);
	return (std::string(ret));

}

bool util::isServiceRunning(const char* serviceName) {
	// Service hdl;
	// Result rc = smGetService(&hdl, serviceName);
	// if (rc == 0)
	// {
	//   serviceClose(&hdl);
	//   return true;
	// };
	// return false;
	u8 tmp = 0;
	SmServiceName service_name = smEncodeName(serviceName);
	Result rc = serviceDispatchInOut(smGetServiceSession(), 65100, service_name, tmp);
	if (R_SUCCEEDED(rc) && tmp & 1)
		return true;
	else
		return false;
}

