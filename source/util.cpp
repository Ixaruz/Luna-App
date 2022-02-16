#include "util.h"

extern const s64 SaveHeaderSize = 0x110;

extern const u64 mainSize = 0x547520 - SaveHeaderSize; //changed in 2.0.0 // GSaveLandOther - Header
extern const u64 GSavePlayerVillagerAccountOffset = 0x1E34C0 - SaveHeaderSize; //changed in 2.0.0
extern const u64 GSavePlayerVillagerAccountSize = 0x48;
extern const u64 GAccountTableOffset = 0x10;
extern const u64 DreamIDOffset = 0x545F50; //changed in 2.0.0
extern const u64 DreamInfoSize = 0x50;


extern const u64 playerSize = 0x36A50 - SaveHeaderSize; //changed in 1.10.0 // GSaveLandOther - Header
extern const u64 playersOffset = 0x7B658; //changed in 1.9.0 //alternatively in the player pointer [[[[main+XXXXXX]+10]+140]+08] you can add 0x8 to 0x140 for each additional player

//taken from NHSE
//*personal.dat*//
extern const u64 PersonalID = 0xAFA8; 
extern const u64 EventFlagsPlayerOffset = 0xAFE0;
extern const u64 ItemCollectBitOffset = 0xA058;
extern const u64 ItemRemakeCollectBitOffset = 0xA7AC;
extern const u64 RecipesOffset = 0x24afc + 0x10; //Bank + 0x10
extern const u64 StorageSizeOffset = playerSize + SaveHeaderSize + 0x18C + (8 * 5000); //absolute file offset //changed in 1.7.0 0x4081C
extern const u64 Pocket1SizeOffset = playerSize + SaveHeaderSize + 0x10 + (8 * 20); //absolute file offset //changed in 1.7.0 0x36B00
extern const u64 ExpandBaggageOffset = 0x36BD8;

//*main.dat*//
extern const u64 houseSize = 0x28A28; //changed in 2.0.0
extern const u64 EventFlagOffset = 0x22ebf0; //changed in 2.0.0 //EventFlagLand
extern const u64 houseLvlOffset = 0x30a6bc; //changed in 2.0.0 //PlayerHouseList
extern const u64 SaveFgOffset = 0x462278; //changed in 2.0.0 //SaveFg
extern const u64 SpecialityFruitOffset = 0x900; //SpecialityFruit



extern const std::vector<u16> MainmenuRecipes = std::vector<u16>{
		0x00A, //flimsy axe
		0x00D, //campfire
		0x015, //flimsy fishing rod
		0x016, //flimsy net
		0x06B, //ladder
};

extern const std::vector<u16> PrettyGoodToolsRecipesRecipes = std::vector<u16>{
		0x10F, //axe
		0x118, //watering can
		0x119, //shovel
		0x11A, //fishing rod
		0x11B, //net
		0x11D, //stone axe
};

extern const std::vector<u16> BeAChefRecipes = std::vector<u16>{
		0x34C, //carrot potage
		0x387, //flour
		0x38A, //sugar
		0x3A4, //tomato curry
		0x3A5, //pumpkin bagel sandwich
		0x3A9, //pancakes
		0x3B1, //grilled sea bass with herbs
		0x410, // stonework kitchen
};

const std::map<u16, u16> TownfruitSmoothiesMap = {
	{2213, 0x38D}, //apple smoothie
	{2214, 0x38E}, //orange smoothie
	{2285, 0x388}, //pear smoothie
	{2286, 0x38C}, //peach smoothie
	{2287, 0x384}, //cherry smoothie
};

/*2.0.X specific stuff*/

extern const std::vector<u64> BID = {
	0xE4BBD879D326A0AD, //2.0.0
	0x8C81A85AA4C1990B, //2.0.1
	0xE5759E5B7E31411B, //2.0.2
	0x205F55C725C16C6F, //2.0.3
	0x372C5EA461D03A7D, //2.0.4
	0x747A5B4CBC530AED, //2.0.5
};

extern const std::vector<u64> VersionPointerOffset = {
	0x4BAEF28, //2.0.0
	0x4BAEF28, //2.0.1
	0x4BAEF28, //2.0.2
	0x4BAEF28, //2.0.3
	0x4C1AD20, //2.0.4
	0x4C1AD20, //2.0.5
};

extern const u32 REV_200_MAIN_SIZE = 0x8F1BB0;
extern const u32 REV_200_PERSONAL_SIZE = 0x6A520;

extern const std::vector<FileHashRegion*> REV_200_MAIN = std::vector<FileHashRegion*> {
		new FileHashRegion(0x000110, 0x1e339c),
		new FileHashRegion(0x1e34b0, 0x36406c),
		new FileHashRegion(0x547630, 0x03693c),
		new FileHashRegion(0x57df70, 0x033acc),
		new FileHashRegion(0x5b1b50, 0x03693c),
		new FileHashRegion(0x5e8490, 0x033acc),
		new FileHashRegion(0x61c070, 0x03693c),
		new FileHashRegion(0x6529b0, 0x033acc),
		new FileHashRegion(0x686590, 0x03693c),
		new FileHashRegion(0x6bced0, 0x033acc),
		new FileHashRegion(0x6f0ab0, 0x03693c),
		new FileHashRegion(0x7273f0, 0x033acc),
		new FileHashRegion(0x75afd0, 0x03693c),
		new FileHashRegion(0x791910, 0x033acc),
		new FileHashRegion(0x7c54f0, 0x03693c),
		new FileHashRegion(0x7fbe30, 0x033acc),
		new FileHashRegion(0x82fa10, 0x03693c),
		new FileHashRegion(0x866350, 0x033acc),
		new FileHashRegion(0x899e20, 0x057d8c),
};

extern const std::vector<FileHashRegion*> REV_200_PERSONAL = std::vector<FileHashRegion*> {
		new FileHashRegion(0x00110, 0x3693c),
		new FileHashRegion(0x36a50, 0x33acc),
};

extern int versionindex = 0;

bool util::findVersionIndex(u64 versionBID) {
	auto p = std::find(BID.begin(), BID.end(), versionBID);
	if (p == BID.end()) return false;
	else {
		versionindex = std::distance(BID.begin(), p);
		printf("current version: 2.0.%d\n", versionindex);
		return true;
	}
}

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
	u16 name[0xB] = { 0 };
	u16 namechar;
	u8 lastchar = 0;

	//0xB - 0x1 bc we dont need the 0x2 to determine the end of the string.
	for (u8 i = 0; i < 0xA; i++) {
		dmntchtReadCheatProcessMemory(playerAddr + PersonalID + 0x4 + (i * 0x2), &namechar, 0x2);
		//make sure we can use this fuck string in a path
		if ((isASCII(namechar) && !isVerboten(namechar)) || namechar == 0) {
			name[i] = namechar;
			lastchar = i;
		}
		else {
			printf("invalid char in island name: 0x%02X\n", namechar);
			name[i] = 0x005F;
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

void util::SetFlag(u8* Data, int bitIndex, u16 value) {
	int offset = (bitIndex >> 3); //if larger than 8, its the next byte
	bitIndex &= 7; // ensure bit access is 0-7
	Data[offset] &= (u8)~(1 << bitIndex);
	Data[offset] |= (u8)((value ? 1 : 0) << bitIndex);
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

void util::overclockSystem(bool enable) {
	if (hosversionBefore(8, 0, 0)) {
		pcvSetClockRate(PcvModule_CpuBus, enable ? 1785 MHz : 1020 MHz);  // Set CPU clock
		pcvSetClockRate(PcvModule_EMC, enable ? 1600 MHz : 1331 MHz);     // Set memory clock
	}
	else {
		ClkrstSession clkrstSession;
		clkrstOpenSession(&clkrstSession, PcvModuleId_CpuBus, 3);
		clkrstSetClockRate(&clkrstSession, enable ? 1785 MHz : 1020 MHz); // Set CPU clock
		clkrstCloseSession(&clkrstSession);

		clkrstOpenSession(&clkrstSession, PcvModuleId_EMC, 3);
		clkrstSetClockRate(&clkrstSession, enable ? 1600 MHz : 1331 MHz); // Set memory clock
		clkrstCloseSession(&clkrstSession);
	}
}