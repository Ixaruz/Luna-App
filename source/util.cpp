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
		util::PrintToNXLink("current version: 2.0.%d\n", versionindex);
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

enum class JapFlag {
	normal,
	small,
	sokuon,
};

struct JapLetter {
	u16 UCS;
	const char* romaji;
	JapFlag flag = JapFlag::normal;

};

const std::vector<JapLetter> JAP = {
	{0x3041, "A", JapFlag::small},
	{0x3042, "A"},
	{0x3043, "I", JapFlag::small},
	{0x3044, "I"},
	{0x3045, "U", JapFlag::small},
	{0x3046, "U"},
	{0x3047, "E", JapFlag::small},
	{0x3048, "E"},
	{0x3049, "O", JapFlag::small},
	{0x304A, "O"},
	{0x304B, "KA"},
	{0x304C, "GA"},
	{0x304D, "KI"},
	{0x304E, "GI"},
	{0x304F, "KU"},
	{0x3050, "GU"},
	{0x3051, "KE"},
	{0x3052, "GE"},
	{0x3053, "KO"},
	{0x3054, "GO"},
	{0x3055, "SA"},
	{0x3056, "ZA"},
	{0x3057, "SHI"},
	{0x3058, "JI"},
	{0x3059, "SU"},
	{0x305A, "ZU"},
	{0x305B, "SE"},
	{0x305C, "ZE"},
	{0x305D, "SO"},
	{0x305E, "ZO"},
	{0x305F, "TA"},
	{0x3060, "DA"},
	{0x3061, "CHI"},
	{0x3062, "JI"},
	{0x3063, "TSU", JapFlag::sokuon},
	{0x3064, "TSU"},
	{0x3065, "ZU"},
	{0x3066, "TE"},
	{0x3067, "DE"},
	{0x3068, "TO"},
	{0x3069, "DO"},
	{0x306A, "NA"},
	{0x306B, "NI"},
	{0x306C, "NU"},
	{0x306D, "NE"},
	{0x306E, "NO"},
	{0x306F, "HA"},
	{0x3070, "BA"},
	{0x3071, "PA"},
	{0x3072, "HI"},
	{0x3073, "BI"},
	{0x3074, "PI"},
	{0x3075, "FU"},
	{0x3076, "BU"},
	{0x3077, "PU"},
	{0x3078, "HE"},
	{0x3079, "BE"},
	{0x307A, "PE"},
	{0x307B, "HO"},
	{0x307C, "BO"},
	{0x307D, "PO"},
	{0x307E, "MA"},
	{0x307F, "MI"},
	{0x3080, "MU"},
	{0x3081, "ME"},
	{0x3082, "MO"},
	{0x3083, "YA", JapFlag::small},
	{0x3084, "YA"},
	{0x3085, "YU", JapFlag::small},
	{0x3086, "YU"},
	{0x3087, "YO", JapFlag::small},
	{0x3088, "YO"},
	{0x3089, "RA"},
	{0x308A, "RI"},
	{0x308B, "RU"},
	{0x308C, "RE"},
	{0x308D, "RO"},
	{0x308E, "WA", JapFlag::small},
	{0x308F, "WA"},
	{0x3090, "WI"},
	{0x3091, "WE"},
	{0x3092, "WO"},
	{0x3093, "N"},
	{0x30A1, "A", JapFlag::small},
	{0x30A2, "A"},
	{0x30A3, "I", JapFlag::small},
	{0x30A4, "I"},
	{0x30A5, "U", JapFlag::small},
	{0x30A6, "U"},
	{0x30A7, "E", JapFlag::small},
	{0x30A8, "E"},
	{0x30A9, "O", JapFlag::small},
	{0x30AA, "O"},
	{0x30AB, "KA"},
	{0x30AC, "GA"},
	{0x30AD, "KI"},
	{0x30AE, "GI"},
	{0x30AF, "KU"},
	{0x30B0, "GU"},
	{0x30B1, "KE"},
	{0x30B2, "GE"},
	{0x30B3, "KO"},
	{0x30B4, "GO"},
	{0x30B5, "SA"},
	{0x30B6, "ZA"},
	{0x30B7, "SHI"},
	{0x30B8, "JI"},
	{0x30B9, "SU"},
	{0x30BA, "ZU"},
	{0x30BB, "SE"},
	{0x30BC, "ZE"},
	{0x30BD, "SO"},
	{0x30BE, "ZO"},
	{0x30BF, "TA"},
	{0x30C0, "DA"},
	{0x30C1, "CHI"},
	{0x30C2, "JI"},
	{0x30C3, "TSU", JapFlag::sokuon},
	{0x30C4, "TSU"},
	{0x30C5, "DU"},
	{0x30C6, "TE"},
	{0x30C7, "DE"},
	{0x30C8, "TO"},
	{0x30C9, "DO"},
	{0x30CA, "NA"},
	{0x30CB, "NI"},
	{0x30CC, "NU"},
	{0x30CD, "NE"},
	{0x30CE, "NO"},
	{0x30CF, "HA"},
	{0x30D0, "BA"},
	{0x30D1, "PA"},
	{0x30D2, "HI"},
	{0x30D3, "BI"},
	{0x30D4, "PI"},
	{0x30D5, "FU"},
	{0x30D6, "BU"},
	{0x30D7, "PU"},
	{0x30D8, "HE"},
	{0x30D9, "BE"},
	{0x30DA, "PE"},
	{0x30DB, "HO"},
	{0x30DC, "BO"},
	{0x30DD, "PO"},
	{0x30DE, "MA"},
	{0x30DF, "MI"},
	{0x30E0, "MU"},
	{0x30E1, "ME"},
	{0x30E2, "MO"},
	{0x30E3, "YA", JapFlag::small},
	{0x30E4, "YA"},
	{0x30E5, "YU", JapFlag::small},
	{0x30E6, "YU"},
	{0x30E7, "YO", JapFlag::small},
	{0x30E8, "YO"},
	{0x30E9, "RA"},
	{0x30EA, "RI"},
	{0x30EB, "RU"},
	{0x30EC, "RE"},
	{0x30ED, "RO"},
	{0x30EE, "WA", JapFlag::small},
	{0x30EF, "WA"},
	{0x30F0, "WI"},
	{0x30F1, "WE"},
	{0x30F2, "WO"},
	{0x30F3, "N"},
	{0x30F4, "VU"},
	{0x30F5, "KA", JapFlag::small},
	{0x30F6, "KE", JapFlag::small},
};

int getelementindex(std::vector<JapLetter> v, u16 val) {
	auto p = std::find_if( //find() ask for a value, use find_if() for condition
		v.begin(),
		v.end(),
		[val](const JapLetter& vi) //you want to compare an item
		{return vi.UCS == val; }
	);
	return (int)(p - v.begin());
}

static bool isJAP(const u16& t) {
	return (t >= 0x3041 && t <= 0x30F6);
}

static bool isKATAKANA(const u16& t) {
	return (t > 0x30A1);
}

std::string util::getIslandNameASCII(u64 playerAddr)
{
	u16 name[0xB*2] = { 0 };
	u16 namechar = 0, lastchar = 0;
	JapLetter lastletter = {0x0000, "\0"};
	u8 lastvalidindex = 0;
	int currentoffset = 0;
	int currentletteroffset = 0;
	static bool lastwassokuon = false;

	//0xB - 0x1 bc we dont need the 0x2 to determine the end of the string.
	for (u8 i = 0; i < 0xA; i++) {
		dmntchtReadCheatProcessMemory(playerAddr + PersonalID + 0x4 + (i * 0x2), &namechar, 0x2);
		//make sure we can use this fuck string in a path
		if ((isASCII(namechar) && !isVerboten(namechar)) || namechar == 0) {
			name[i + currentoffset] = namechar;
		}
		else {
			if (isJAP(namechar)) {
				currentletteroffset = 0;
				JapLetter currentletter = JAP[getelementindex(JAP, namechar)];
				if (currentletter.flag != JapFlag::sokuon) {
					const char* romeqiv = currentletter.romaji;
					//chouon deez nuts
					if (i != 0 || currentoffset != 0) {
						//check if the last character isn't the current vowel (ga + a = gaa)
						if (name[i + currentoffset - 1] != *romeqiv) {
							currentoffset -= (int)currentletter.flag;
						}
						//special cases of ya, yu, yo with shi, chi and ji (= sha, shu, sho)
						if (currentletter.flag == JapFlag::small) {
							if (currentletter.UCS >= 0x30E3 && currentletter.UCS <= 0x30E7) {
								if (lastletter.UCS == 0x30B7 || lastletter.UCS == 0x30B8 || lastletter.UCS == 0x30C1 || lastletter.UCS == 0x30C2) {
									//remove first letter (being 'y')
									currentletteroffset = 1;
								}
							}
							if (currentletter.UCS >= 0x3083 && currentletter.UCS <= 0x3087) {
								if (lastletter.UCS == 0x3057 || lastletter.UCS == 0x3058 || lastletter.UCS == 0x3061 || lastletter.UCS == 0x3062) {
									//remove first letter (being 'y')
									currentletteroffset = 1;
								}
							}
						}
					}
					//sokuon deez nuts
					//take the consonant and put it in the last letter spot
					if (lastwassokuon) {
						name[i + currentoffset - 1] = *(romeqiv + currentletteroffset);
						lastwassokuon = false;
					}
					name[i + currentoffset] = *(romeqiv + currentletteroffset);
					//this runs when the romaji equivalent is longer than 1 character
					for (size_t j = 1; j < std::strlen(romeqiv + currentletteroffset); j++) {
						currentoffset++;
						name[i + currentoffset] = *(romeqiv + currentletteroffset + j);
					}
				}
				else {
					lastwassokuon = true;
				}
				lastletter = currentletter;
			}
			//chouon deez nuts 
			//the elongation symbol is only used in KATAKANA
			else if (namechar == 0x30FC && isJAP(lastchar) && isKATAKANA(lastchar)) {
				name[i + currentoffset] = name[i + currentoffset - 1];
				
			}
			else {
				util::PrintToNXLink("invalid letter: 0x%04X\n", namechar);
				name[i + currentoffset] = 0x005F;
			}
		}
		lastchar = namechar;
		lastvalidindex = i + currentoffset;
	}
	
	//make sure there is no space on path ends
	if (name[lastvalidindex] == 0x20) {
		name[lastvalidindex] = 0x0000;
	}

	//nullterminator pain
	u8 name_string[0x16*2] = { 0 };
	//pain
	utf16_to_utf8(name_string, name, sizeof(name_string) / sizeof(u8));

	for (int i = 0; i < sizeof(name_string); i++) {
		util::PrintToNXLink("%c", name_string[i]);
	}
	util::PrintToNXLink("\n");

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
		util::PrintToNXLink("Memory Read failed.\n");
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

u32 util::GetWeatherRandomSeed(u64 mainAddr) {
	u32 randomweatherseed;
	dmntchtReadCheatProcessMemory(mainAddr + 0x1e35f0 + 0x18, &randomweatherseed, 0x4);
	return randomweatherseed;
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

void util::PrintToNXLink(const char* format, ...) {
#if DEBUG_PRINTF
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
#endif
}