#pragma once
#include "util.h"
#include "helpers/file/file.h"
#include <SaveCrypto.hpp>
#include <MurmurHash3.hpp>
#include "luna.h"
#include "ui.hpp"

namespace Dump {

	namespace {
		/* Menu related */
		std::shared_ptr<dbk::DumpingMenu> g_dumping_menu;
		float *g_progress_percent;
		float g_progress_percent_last_function;
		bool *g_enable_buttons;
		dbk::DumpingMenu::DumpState *g_dumping_state;
		
		/* FS related */
		std::string g_TemplateIn = std::string(LUNA_TEMPLATE_DIR);
		std::string g_pathOut = std::string(LUNA_DUMP_DIR);
		FsFileSystem g_fsSdmc;
		int g_datcount = 0;

		static std::vector<std::string> g_pathFilter;

		/* Memory/Dumping related */
		size_t g_AccountTableSize = GAccountTableOffset + (0x8 * GSavePlayerVillagerAccountSize);

		u64 g_mainAddr = 0;
		u64 g_playerAddr = 0;
		bool g_players[0x8] = { false };
		u8* g_AccountTableBuffer = nullptr;
		size_t g_bufferSize = BUFF_SIZE;
		u8* g_buffer = nullptr;

		size_t g_RecipeBookSize = 0x100 * 4; //each recipe has 4 flags; there are 0x800max recipes, there are basically "4 books", 0x100 known recipes, 0x100 made recipes, 0x100 new flag, 0x100 favorite flag. IT DOESNT HAVE ALL 4 FLAGS NEXT TO EACH OTHER
		u8 *g_RecipeBook = nullptr;

		void addPathFilter(const std::string& _path)
		{
			g_pathFilter.push_back(_path);
		}

		bool pathIsFiltered(const std::string& _path)
		{
			if (g_pathFilter.empty())
				return false;

			for (std::string& _p : g_pathFilter)
			{
				if (_path == _p)
					return true;
			}

			return false;
		}

		void freePathFilters()
		{
			g_pathFilter.clear();
		}
	}

	void addProgress(float progress) {
		*g_progress_percent = g_progress_percent_last_function + progress;
		g_progress_percent_last_function = *g_progress_percent;
	}

	void setProgress(float progress) {
		*g_progress_percent = progress;
		g_progress_percent_last_function = *g_progress_percent;
	}

	void enableButton() {
		*g_enable_buttons = true;
		setProgress(1.0f);
		*g_dumping_state = dbk::DumpingMenu::DumpState::End;
	}

	void checkPlayers() {
		for (u8 i = 0; i < 8; i++) {
			u64 offset = i * GSavePlayerVillagerAccountSize;
			u128 AccountUID = 0;
			util::PrintToNXLink("%lX\n", g_mainAddr + GSavePlayerVillagerAccountOffset + offset);
			dmntchtReadCheatProcessMemory(g_mainAddr + GSavePlayerVillagerAccountOffset + offset, &AccountUID, 0x10);
			if (AccountUID != 0) g_players[i] = true;
			u64 part1 = (u64)(AccountUID >> 64), part2 = (u64)(AccountUID & 0xFFFFFFFFFFFFFFFF);
			util::PrintToNXLink("%lX%lX\n", part1, part2);
		}
	}

	void setPathFilter() {
		for (u8 i = 0; i < 8; i++) {
			if (!g_players[i]) addPathFilter(g_TemplateIn + "Villager" + std::to_string(i));
		}
	}

	u8 getPlayerNumber() {
		u8 playernum = 0;
		for (u8 i = 0; i < 8; i++) {
			if (g_players[i]) playernum++;
		}
		return playernum;
	}

	void getAccountTable() {
		FsFile md;
		u64 bytesread;
		static char path[FS_MAX_PATH] = { 0 };
		std::snprintf(path, FS_MAX_PATH, (g_pathOut + "main.dat").c_str());
		fsFsOpenFile(&g_fsSdmc, path, FsOpenMode_Read, &md);
		g_AccountTableBuffer = new u8[g_AccountTableSize];
		fsFileRead(&md, SaveHeaderSize + GSavePlayerVillagerAccountOffset - GAccountTableOffset, g_AccountTableBuffer, g_AccountTableSize, FsReadOption_None, &bytesread);
		fsFileClose(&md);
	}

	void DecryptPair(const std::string &dataPathIn, const std::string &dataPathOut) {
		FsFile hd;
		FsFile md;
		static char path[FS_MAX_PATH];
		//clear our path buffer or bad things will happen
		memset(path, 0, FS_MAX_PATH);
		std::snprintf(path, FS_MAX_PATH, dataPathIn.c_str());
		fsFsOpenFile(&g_fsSdmc, path, FsOpenMode_Read, &md);
		s64 datasize = 0;
		fsFileGetSize(&md, &datasize);

		std::string headerPathIn = dataPathIn.substr(0, dataPathIn.find_last_of('.')) + "Header.dat";

		util::PrintToNXLink("Data : \t\t" + dataPathIn + "\n");
		util::PrintToNXLink("Headers : \t" + headerPathIn + "\n");

		//clear our path buffer or bad things will happen
		memset(path, 0, FS_MAX_PATH);
		std::snprintf(path, FS_MAX_PATH, headerPathIn.c_str());
		fsFsOpenFile(&g_fsSdmc, path, FsOpenMode_Read, &hd);
		s64 headersize = 0;
		fsFileGetSize(&hd, &headersize);


		GSaveVersion headerData;
		u8* encData = new u8[datasize];
		u64 bytesread = 0;

		fsFileRead(&hd, 0, &headerData, headersize, FsReadOption_None, &bytesread);
		fsFileClose(&hd);
		fsFileRead(&md, 0, encData, datasize, FsReadOption_None, &bytesread);
		fsFileClose(&md);

		//util::PrintToNXLink("dataSize for Sluttz Cocktober: %ld\n", datasize);

		SaveCrypto::Crypt(headerData, encData, datasize);

		FsFile newmd;
		//clear our path buffer or bad things will happen
		memset(path, 0, FS_MAX_PATH);
		std::snprintf(path, FS_MAX_PATH, dataPathOut.c_str());
		if (access(path, F_OK) != -1) {
			fsFsDeleteFile(&g_fsSdmc, path);	
		}
		fsFsCreateFile(&g_fsSdmc, path, datasize, 0);
		fsFsOpenFile(&g_fsSdmc, path, FsOpenMode_Write, &newmd);
		fsFileWrite(&newmd, 0, encData, datasize, FsWriteOption_Flush);
		fsFileClose(&newmd);
		delete encData;
	}

	void EncryptPair(const std::string &dataPathIn, const std::string &dataPathOut, u32 tick) {
		FsFile md;
		static char path[FS_MAX_PATH];
		//clear our path buffer or bad things will happen
		memset(path, 0, FS_MAX_PATH);
		std::snprintf(path, FS_MAX_PATH, dataPathIn.c_str());
		fsFsOpenFile(&g_fsSdmc, path, FsOpenMode_Read, &md);
		s64 datasize = 0;
		fsFileGetSize(&md, &datasize);

		util::PrintToNXLink("Data : \t\t" + dataPathIn + "\n");

		u8 *encData = new u8[datasize];
		u64 bytesread = 0;

		fsFileRead(&md, 0, encData, datasize, FsReadOption_None, &bytesread);
		fsFileClose(&md);

		GSaveVersion headerData;
		//copy the first 0x100 bytes of metadata to the headerdata
		memcpy(&headerData, encData, sizeof(headerData) - sizeof(headerData.headerCrypto));
		std::string headerPathOut = dataPathOut.substr(0, dataPathOut.find_last_of('.')) + "Header.dat";

		SaveCrypto::RegenHeaderCrypto(headerData, tick);
		SaveCrypto::Crypt(headerData, encData, datasize);

		FsFile newmd;
		FsFile newhd;
		//clear our path buffer or bad things will happen
		memset(path, 0, FS_MAX_PATH);
		std::snprintf(path, FS_MAX_PATH, dataPathOut.c_str());
		if (access(path, F_OK) != -1) {
			fsFsDeleteFile(&g_fsSdmc, path);	
		}
		fsFsCreateFile(&g_fsSdmc, path, datasize, 0);
		fsFsOpenFile(&g_fsSdmc, path, FsOpenMode_Write, &newmd);
		fsFileWrite(&newmd, 0, encData, datasize, FsWriteOption_Flush);
		fsFileClose(&newmd);

		//clear our path buffer or bad things will happen
		memset(path, 0, FS_MAX_PATH);
		std::snprintf(path, FS_MAX_PATH, headerPathOut.c_str());
		if (access(path, F_OK) != -1) {
			fsFsDeleteFile(&g_fsSdmc, path);
		}
		fsFsCreateFile(&g_fsSdmc, path, sizeof(decltype(headerData)), 0);
		fsFsOpenFile(&g_fsSdmc, path, FsOpenMode_Write, &newhd);
		fsFileWrite(&newhd, 0, &headerData, sizeof(decltype(headerData)), FsWriteOption_Flush);
		fsFileClose(&newhd);
		delete encData;
	}

	void Hash(const std::string& dataPathIn) {
		
		size_t slPos = dataPathIn.find_last_of('/');
		std::string fileName = dataPathIn.substr(slPos + 1, std::string::npos);

		//the files we edited
		if (fileName != "main.dat" && fileName != "personal.dat") return;

		FsFile md;
		static char path[FS_MAX_PATH];
		//clear our path buffer or bad things will happen
		memset(path, 0, FS_MAX_PATH);
		std::snprintf(path, FS_MAX_PATH, dataPathIn.c_str());
		fsFsOpenFile(&g_fsSdmc, path, FsOpenMode_Read | FsOpenMode_Write, &md);
		s64 datasize = 0;
		fsFileGetSize(&md, &datasize);
		u8* Data = new u8[datasize];
		u64 bytesread = 0;

		fsFileRead(&md, 0, Data, datasize, FsReadOption_None, &bytesread);
		
		if (fileName == "main.dat") {
			for (auto &h : REV_200_MAIN) {
				MurmurHash3::Update(Data, h->HashOffset, h->getBeginOffset(), h->Size);
			}
		}

		if (fileName == "personal.dat") {
			for (auto &h : REV_200_PERSONAL) {
				MurmurHash3::Update(Data, h->HashOffset, h->getBeginOffset(), h->Size);
			}
		}

		fsFileWrite(&md, 0, Data, datasize, FsWriteOption_Flush);
		fsFileClose(&md);
	}

	void handleDecryption(const std::string &in, const std::string &out, int datcount) {
		fs::dirList list(in);
		unsigned int listcount = list.getCount();
		for (unsigned int i = 0; i < listcount; i++)
		{
			if (pathIsFiltered(in + list.getItem(i)))
				continue;

			if (list.isDir(i))
			{
				util::PrintToNXLink("isDir : %d\n", list.isDir(i));
				std::string newIn = in + list.getItem(i) + "/";
				std::string newOut = out + list.getItem(i) + "/";
				mkdir(newOut.c_str(), 0777);
				//Result rc = fsFsCreateDirectory(&g_fsSdmc, newOut.c_str());
				//util::PrintResultToNXLink(rc);
				handleDecryption(newIn, newOut, datcount);
			}
			//skip over landname.dat
			else if (list.getItem(i) == "landname.dat") {
				util::PrintToNXLink("found landname.dat\n");
				std::string fullIn = in + list.getItem(i);
				std::string fullOut = out + list.getItem(i);
				fs::copyFile(&g_fsSdmc, fullIn, fullOut);
				continue;
			}
			else if (list.getItemExt(i) == "dat") {
				std::string dataPathIn = in + list.getItem(i);
				std::string dataPathOut = out + list.getItem(i);
				if ((util::getFilename(dataPathIn).find("Header") == std::string::npos)) {
					DecryptPair(dataPathIn, dataPathOut);
					addProgress(0.20f / datcount);
				}
			}
		}
	}

	void handleEncryption( const std::string &in, const std::string &out, u32 tick, int datcount) {
		fs::dirList list(in);
		unsigned int listcount = list.getCount();
		for (unsigned int i = 0; i < listcount; i++)
		{
			if (list.isDir(i))
			{
				if (pathIsFiltered(in + list.getItem(i)))
					continue;

				util::PrintToNXLink("isDir : %d\n", list.isDir(i));
				std::string newIn = in + list.getItem(i) + "/";
				std::string newOut = out + list.getItem(i) + "/";
				mkdir(newOut.c_str(), 0777);
				//Result rc = fsFsCreateDirectory(&g_fsSdmc, newOut.c_str());
				//util::PrintResultToNXLink(rc);
				handleEncryption(newIn, newOut, tick, datcount);
			}
			//skip over landname.dat
			else if (list.getItem(i) == "landname.dat") {
				util::PrintToNXLink("found landname.dat\n");
				std::string fullIn = in + list.getItem(i);
				std::string fullOut = out + list.getItem(i);
				fs::copyFile(&g_fsSdmc, fullIn, fullOut);
				continue;
			}
			else if (list.getItemExt(i) == "dat") {
				std::string dataPathIn = in + list.getItem(i);
				std::string dataPathOut = out + list.getItem(i);
				if ((util::getFilename(dataPathIn).find("Header") == std::string::npos)) {
					Hash(dataPathIn);
					EncryptPair(dataPathIn, dataPathOut, tick);
					addProgress(0.40f / datcount);
				}
			}
		}
	}

	int countDATFilesRecursively(const std::string& in) {
		int filecount = 0;
		fs::dirList list(in);
		unsigned int listcount = list.getCount();
		for (unsigned int i = 0; i < listcount; i++)
		{
			if (list.isDir(i))
			{
				if (pathIsFiltered(in + list.getItem(i)))
					continue;

				std::string newIn = in + list.getItem(i) + "/";
				filecount += countDATFilesRecursively(newIn);
			}
			//skip over landname.dat
			else if (list.getItem(i) == "landname.dat") {
				continue;
			}
			else if (list.getItemExt(i) == "dat") {
				std::string dataPathIn = in + list.getItem(i);
				if ((util::getFilename(dataPathIn).find("Header") == std::string::npos)) {
					filecount++;
				}
			}
		}
		return filecount;
	}

	/* Setting up all kinds of variables for dumping */
	void Setup(std::shared_ptr<dbk::DumpingMenu> menu, float *progress_percent, bool *enable_buttons, dbk::DumpingMenu::DumpState *dumping_state) {
		g_dumping_menu = menu;
		g_progress_percent = progress_percent;
		g_enable_buttons = enable_buttons;
		g_dumping_state = dumping_state;
		fsOpenSdCardFileSystem(&g_fsSdmc);
		g_mainAddr = util::FollowPointerMain(VersionPointerOffset[versionindex], 0x10, 0x130, 0x10, 0xFFFFFFFFFFFFFFFF);
		if (g_mainAddr == 0x00) {
			g_dumping_menu->LogAddLine("Error: mainAddr");
			util::PrintToNXLink("Error: mainAddr\n");
#if !DEBUG_UI
			enableButton();
			return;
#endif
		}
		g_playerAddr = util::FollowPointerMain(VersionPointerOffset[versionindex], 0x10, 0x140, 0x08, 0xFFFFFFFFFFFFFFFF);
		if (g_playerAddr == 0x00) {
			g_dumping_menu->LogAddLine("Error: playerAddr");
			util::PrintToNXLink("Error: playerAddr\n");
#if !DEBUG_UI
			enableButton();
			return;
#endif
		}
	
#if !DEBUG_UI
		checkPlayers();
		freePathFilters();
		setPathFilter();
		g_datcount = countDATFilesRecursively(g_TemplateIn);

		TimeCalendarTime dumpdreamtime = util::getDreamTime(g_mainAddr);
		char dreamtime[128];
		const char* date_format = "%02d.%02d.%04d @ %02d-%02d";
		sprintf(dreamtime, date_format, dumpdreamtime.day, dumpdreamtime.month, dumpdreamtime.year, dumpdreamtime.hour, dumpdreamtime.minute);

		std::string strislandname = util::getIslandNameASCII(g_playerAddr);

		g_pathOut += strislandname.empty() ? "[DA-" + util::getDreamAddrString(g_mainAddr) + "]" : ("[DA-" + util::getDreamAddrString(g_mainAddr) + "] " + strislandname);
		g_pathOut += "/";
		util::PrintToNXLink((g_pathOut + "\n").c_str());
		//Result rc = fsFsCreateDirectory(&g_fsSdmc, g_pathOut.c_str());
		//util::PrintResultToNXLink(rc);
		mkdir(g_pathOut.c_str(), 0777);
		if (!strislandname.empty()) g_dumping_menu->LogAddLine("DA-" + util::getDreamAddrString(g_mainAddr) + " " + strislandname);
		else g_dumping_menu->LogAddLine("DA-" + util::getDreamAddrString(g_mainAddr));
		g_pathOut += std::string(dreamtime) + "/";
		util::PrintToNXLink((g_pathOut + "\n").c_str());
		//idk why, but fsFsCreateDirectory REALLY doesn't like the date_format directory name, no matter how i formatted it. so i resort to mkdir, 
		//even though it's kinda nasty to use 2 file system implementations at once
		mkdir(g_pathOut.c_str(), 0777);
		//rc = fsFsCreateDirectory(&g_fsSdmc, g_pathOut.c_str());
		//util::PrintResultToNXLink(rc);
		//g_pathOut += "/";
		g_dumping_menu->LogAddLine(std::string(dreamtime));
		util::PrintToNXLink(dreamtime);
		u32 randweather = util::GetWeatherRandomSeed(g_mainAddr);
		util::PrintToNXLink("randweatherseed: %d", randweather);
		g_dumping_menu->LogAddLine("Weather Seed: " + std::to_string(util::GetWeatherRandomSeed(g_mainAddr)));
		addProgress(0.10f);
		*g_dumping_state = dbk::DumpingMenu::DumpState::NeedsDecrypt;
#else
		enableButton();
#endif
	}

	void Decrypt() {
		g_dumping_menu->LogAddLine("Decrypting save files...");
		float progress = g_progress_percent_last_function;
		handleDecryption(g_TemplateIn, g_pathOut, g_datcount);
		setProgress(progress + 0.20f);
		g_dumping_menu->LogEditLastElement("Decrypting save files: successful");
#if !DEBUG_UI
		*g_dumping_state = dbk::DumpingMenu::DumpState::NeedsRW;
#else
		enableButton();
#endif
	}

	void RWData() {
		getAccountTable();

		static char pathbuffer[FS_MAX_PATH] = { 0 };
		g_buffer = new u8[BUFF_SIZE];

		/* MAIN */
		FsFile md;

		std::snprintf(pathbuffer, FS_MAX_PATH, (g_pathOut + "main.dat").c_str());
		fsFsOpenFile(&g_fsSdmc, pathbuffer, FsOpenMode_Write, &md);
		
		//reset size in-case it got changed in the latter for loop
		g_bufferSize = BUFF_SIZE;
		float progress = g_progress_percent_last_function;
		g_dumping_menu->LogAddLine("Copying island...");
		u64 offset;
		for (offset = 0; offset < mainSize; offset += g_bufferSize) {
			if (g_bufferSize > mainSize - offset)
				g_bufferSize = mainSize - offset;

			setProgress(progress + (0.10f * (float)offset / mainSize));

			dmntchtReadCheatProcessMemory(g_mainAddr + offset, g_buffer, g_bufferSize);
			fsFileWrite(&md, SaveHeaderSize + offset, g_buffer, g_bufferSize, FsWriteOption_Flush);
		}
		setProgress(progress + 0.10f);
		g_dumping_menu->LogEditLastElement("Copying island: successful");
		util::PrintToNXLink("wrote main.dat\n");
		fsFileClose(&md);

		/* LANDNAME */
		FsFile ld;
		//clear our path buffer or bad things will happen
		memset(pathbuffer, 0, FS_MAX_PATH);
		std::snprintf(pathbuffer, FS_MAX_PATH, std::string(g_pathOut + "landname.dat").c_str());
		u16 islandname[0xB];
		memcpy(islandname, util::getIslandName(g_playerAddr).name, sizeof(islandname));
		//in case user doesn't submit a valid landname.dat file or the file at all
		fsFsDeleteFile(&g_fsSdmc, pathbuffer);
		fsFsCreateFile(&g_fsSdmc, pathbuffer, 0xB * sizeof(u16), 0);
		fsFsOpenFile(&g_fsSdmc, pathbuffer, FsOpenMode_Write, &ld);
		fsFileWrite(&ld, 0, &islandname, 0x16, FsWriteOption_Flush);
		fsFileClose(&ld);

		/* PLAYERS */
		FsFile pd;

		progress = g_progress_percent_last_function;
		g_dumping_menu->LogAddLine("Players to be copied: " + std::to_string(getPlayerNumber()));
		for (u8 i = 0; i < 8; i++) {
			/* If there is no player, dont even bother */
			if (!g_players[i]) continue;

			std::string player = "Villager" + std::to_string(i) + "/";
			std::string currentplayer = g_pathOut + player;
			//clear our path buffer or bad things will happen
			memset(pathbuffer, 0, FS_MAX_PATH);
			std::snprintf(pathbuffer, FS_MAX_PATH, std::string(currentplayer + "personal.dat").c_str());
			fsFsOpenFile(&g_fsSdmc, pathbuffer, FsOpenMode_Write, &pd);

			//reset size in-case it got changed in the latter for loop
			g_bufferSize = BUFF_SIZE;
			g_dumping_menu->LogAddLine("Copying player " + std::to_string(i + 1) + "...");
			for (offset = 0; offset < playerSize; offset += g_bufferSize) {
				if (g_bufferSize > playerSize - offset)
					g_bufferSize = playerSize - offset;

				dmntchtReadCheatProcessMemory(g_playerAddr + offset + (i * playersOffset), g_buffer, g_bufferSize);
				fsFileWrite(&pd, SaveHeaderSize + offset, g_buffer, g_bufferSize, FsWriteOption_Flush);
			}	
			g_dumping_menu->LogEditLastElement("Copying player " + std::to_string(i + 1) + ": successful");
			fsFileClose(&pd);
			util::PrintToNXLink("wrote personal.dat\n");	
			addProgress(0.10f / getPlayerNumber());
		}
		setProgress(progress + 0.10f);

#if !DEBUG_UI
		*g_dumping_state = dbk::DumpingMenu::DumpState::NeedsFix;
#else
		enableButton();
#endif
	}

	void FixMain() {
		static char pathbuffer[FS_MAX_PATH] = { 0 };
		FsFile md;
		std::snprintf(pathbuffer, FS_MAX_PATH, (g_pathOut + "main.dat").c_str());
		fsFsOpenFile(&g_fsSdmc, pathbuffer, FsOpenMode_Write, &md);

		g_dumping_menu->LogAddLine("Applying fixes to main...");
		u16 IsDreamingBed = 0; //346
		u16 TapDreamEnable = 1; //354
		u16 EnableMyDream = 0; //362
		u16 DreamUploadPlayerHaveCreatorID = 0; //364

		fsFileWrite(&md, SaveHeaderSize + EventFlagOffset + (346 * 2), &IsDreamingBed, sizeof(u16), FsWriteOption_Flush);
		fsFileWrite(&md, SaveHeaderSize + EventFlagOffset + (354 * 2), &TapDreamEnable, sizeof(u16), FsWriteOption_Flush);
		fsFileWrite(&md, SaveHeaderSize + EventFlagOffset + (362 * 2), &EnableMyDream, sizeof(u16), FsWriteOption_Flush);
		fsFileWrite(&md, SaveHeaderSize + EventFlagOffset + (364 * 2), &DreamUploadPlayerHaveCreatorID, sizeof(u16), FsWriteOption_Flush);

		/*
		util::PrintToNXLink("Account Table Buffer:\n");
		for (int i = 0; i < g_AccountTableSize; i++) {
			if (((i+1) % 16) == 0) {
				util::PrintToNXLink("%02X\n", g_AccountTableBuffer[i]);
			}
			else {
				util::PrintToNXLink("%02X ", g_AccountTableBuffer[i]);
			}
		}
		util::PrintToNXLink("\n");
		*/

		//write AccountUID linkage (for Nintendo Switch Online)
		for (u8 i = 0; i < 8; i++) {
			if (g_players[i]) {
				u128 AccountUID;
				memcpy(&AccountUID, g_AccountTableBuffer + 0x10 + (i * 0x48), 0x10);
				u64 part1 = (u64)(AccountUID >> 64), part2 = (u64)(AccountUID & 0xFFFFFFFFFFFFFFFF);
				util::PrintToNXLink("wrote UID: %lX%lX\n", part1, part2);
				fsFileWrite(&md, SaveHeaderSize + GSavePlayerVillagerAccountOffset + (i * 0x48), g_AccountTableBuffer + 0x10 + (i * 0x48), 0x10, FsWriteOption_Flush);
			}
		}
		//remove DreamInfo in dumped file
		u8 DreamInfoBuffer[DreamInfoSize] = { 0 };
		fsFileWrite(&md, SaveHeaderSize + DreamIDOffset, DreamInfoBuffer, DreamInfoSize, FsWriteOption_Flush);

		g_dumping_menu->LogEditLastElement("Applying fixes to main: successful");
		fsFileClose(&md);
	}

	void FixPlayers() {
		static char pathbuffer[FS_MAX_PATH] = { 0 };
		FsFile pd;
		u64 bytesread = 0;

		for (u8 i = 0; i < 8; i++) {
			/* If there is no player, dont even bother */
			if (!g_players[i]) continue;

			std::string player = "Villager" + std::to_string(i) + "/";
			std::string currentplayer = g_pathOut + player;
			//clear our path buffer or bad things will happen
			memset(pathbuffer, 0, FS_MAX_PATH);
			std::snprintf(pathbuffer, FS_MAX_PATH, std::string(currentplayer + "personal.dat").c_str());
			fsFsOpenFile(&g_fsSdmc, pathbuffer, FsOpenMode_Read | FsOpenMode_Write, &pd);

			int applyingFixesLogEntryIndex = g_dumping_menu->LogAddLine("Applying fixes to player " + std::to_string(i + 1) + "...");
			u8 houselvl = 0;
			u16 BuiltTownOffice = 0; //59

			//using default sizes
			u32 storageSize = 80;
			u32 pocket1Size = 0;

			u8 ReceivedItemPocket30; //9052
			u8 ReceivedItemPocket40; //11140
			u8 ExpandBaggage = 0;

			u16 UpgradePocket30 = 0; //669
			u16 UpgradePocket40 = 0; //670
			u16 SellPocket40 = 0; //672

			u8 HairStyles[sizeof(u8)]; //9049 - 9051
			u8 StylishHairStyles[sizeof(u8)]; //13767
			u8 PrettyGoodToolsRecipes[sizeof(u8)]; //9221
			u8 ToolRingItsEssential[sizeof(u8)]; //9616
			u8 PermitsandLicenses[sizeof(u16)]; //8773 - 8781
			u8 CustomDesignPathPermit[sizeof(u8)]; //9771
			u8 CustomDesignProEditor[sizeof(u8)]; //12185
			u8 CustomDesignProEditorPlus[sizeof(u8)]; //13195
			u8 HalloweenCharacterColors[sizeof(u8)]; //13256 - 13258
			/// <summary>
			/// 14691-14695
			/// Be a Chef! DIY Recipes+
			/// Custom Designs Patterns +
			///	Pro Decorating License
			///	Custom Fencing in a Flash
			///	Pro Camera App
			/// </summary>
			u8 Update200RecipeLicenseApp[sizeof(u8)];
			u8 IslandLife101Service[sizeof(u8)]; //14796
			u8 Top4FabHairstyles[sizeof(u8)]; //14829

			u16 HairStyleColor[3] = { 0 };
			u16 AddHairStyle4; //1219
			u16 ItemRingEnable; //610
			u16 GetLicenses[9] = { 0 };
			u16 GetLicenseGrdMydesign; //644
			u16 UnlockMyDesignProCategory; //714
			u16 P2_CreatedAfterOwlMoving; //759
			u16 OwlGotDiyRecipe; //760
			u16 UnlockMydesignPro2; //1172
			u16 AddHalloweenColor[3] = { 0 }; //1142-1144
			u16 MainmenuRecipe; //171
			u16 MainmenuRecipe_v2; //1410
			u16 MainmenuMydesignPatternPlus; //1448
			u16 MainmenuCamera1stPersonView; //1450
			u16 UnlockInterior_CeilingFurniture; //1452
			u16 FenceRemakeEnable; //1453
			u16 RcoStorageExpansion_v200_AddLevel; //1458
			u16 MainmenuTips; //1538
			u16 UnlockPlayerHair37to47[11] = { 0 }; //1567-1577

			g_RecipeBook = new u8[g_RecipeBookSize];

			dmntchtReadCheatProcessMemory(g_mainAddr + houseLvlOffset + (i * houseSize), &houselvl, sizeof(u8));
			dmntchtReadCheatProcessMemory(g_mainAddr + EventFlagOffset + (59 * 2), &BuiltTownOffice, sizeof(u16));

			dmntchtReadCheatProcessMemory(g_playerAddr + (i * playersOffset) + EventFlagsPlayerOffset + (559 * 2), &HairStyleColor, sizeof(HairStyleColor));
			dmntchtReadCheatProcessMemory(g_playerAddr + (i * playersOffset) + EventFlagsPlayerOffset + (1219 * 2), &AddHairStyle4, sizeof(u16));
			dmntchtReadCheatProcessMemory(g_playerAddr + (i * playersOffset) + EventFlagsPlayerOffset + (610 * 2), &ItemRingEnable, sizeof(u16));
			dmntchtReadCheatProcessMemory(g_playerAddr + (i * playersOffset) + EventFlagsPlayerOffset + (565 * 2), &GetLicenses, sizeof(GetLicenses));
			dmntchtReadCheatProcessMemory(g_playerAddr + (i * playersOffset) + EventFlagsPlayerOffset + (644 * 2), &GetLicenseGrdMydesign, sizeof(u16));
			dmntchtReadCheatProcessMemory(g_playerAddr + (i * playersOffset) + EventFlagsPlayerOffset + (714 * 2), &UnlockMyDesignProCategory, sizeof(u16));
			dmntchtReadCheatProcessMemory(g_playerAddr + (i * playersOffset) + EventFlagsPlayerOffset + (759 * 2), &P2_CreatedAfterOwlMoving, sizeof(u16));
			dmntchtReadCheatProcessMemory(g_playerAddr + (i * playersOffset) + EventFlagsPlayerOffset + (760 * 2), &OwlGotDiyRecipe, sizeof(u16));
			dmntchtReadCheatProcessMemory(g_playerAddr + (i * playersOffset) + EventFlagsPlayerOffset + (1172 * 2), &UnlockMydesignPro2, sizeof(u16));
			dmntchtReadCheatProcessMemory(g_playerAddr + (i * playersOffset) + EventFlagsPlayerOffset + (1142 * 2), &AddHalloweenColor, sizeof(AddHalloweenColor));
			dmntchtReadCheatProcessMemory(g_playerAddr + (i * playersOffset) + EventFlagsPlayerOffset + (171 * 2), &MainmenuRecipe, sizeof(u16));
			dmntchtReadCheatProcessMemory(g_playerAddr + (i * playersOffset) + EventFlagsPlayerOffset + (1410 * 2), &MainmenuRecipe_v2, sizeof(u16));
			dmntchtReadCheatProcessMemory(g_playerAddr + (i * playersOffset) + EventFlagsPlayerOffset + (1448 * 2), &MainmenuMydesignPatternPlus, sizeof(u16));
			dmntchtReadCheatProcessMemory(g_playerAddr + (i * playersOffset) + EventFlagsPlayerOffset + (1450 * 2), &MainmenuCamera1stPersonView, sizeof(u16));
			dmntchtReadCheatProcessMemory(g_playerAddr + (i * playersOffset) + EventFlagsPlayerOffset + (1452 * 2), &UnlockInterior_CeilingFurniture, sizeof(u16));
			dmntchtReadCheatProcessMemory(g_playerAddr + (i * playersOffset) + EventFlagsPlayerOffset + (1453 * 2), &FenceRemakeEnable, sizeof(u16));
			dmntchtReadCheatProcessMemory(g_playerAddr + (i * playersOffset) + EventFlagsPlayerOffset + (1458 * 2), &RcoStorageExpansion_v200_AddLevel, sizeof(u16));
			dmntchtReadCheatProcessMemory(g_playerAddr + (i * playersOffset) + EventFlagsPlayerOffset + (1538 * 2), &MainmenuTips, sizeof(u16));
			dmntchtReadCheatProcessMemory(g_playerAddr + (i * playersOffset) + EventFlagsPlayerOffset + (1567 * 2), &UnlockPlayerHair37to47, sizeof(UnlockPlayerHair37to47));

			fsFileRead(&pd, SaveHeaderSize + playerSize + ItemCollectBitOffset + (0x2359 / 8), HairStyles, sizeof(u8), FsReadOption_None, &bytesread);
			fsFileRead(&pd, SaveHeaderSize + playerSize + ItemCollectBitOffset + (0x35C6 / 8), StylishHairStyles, sizeof(u8), FsReadOption_None, &bytesread);
			fsFileRead(&pd, SaveHeaderSize + playerSize + ItemCollectBitOffset + (0x2405 / 8), PrettyGoodToolsRecipes, sizeof(u8), FsReadOption_None, &bytesread);
			fsFileRead(&pd, SaveHeaderSize + playerSize + ItemCollectBitOffset + (0x2590 / 8), ToolRingItsEssential, sizeof(u8), FsReadOption_None, &bytesread);
			fsFileRead(&pd, SaveHeaderSize + playerSize + ItemCollectBitOffset + (0x2245 / 8), PermitsandLicenses, sizeof(u16), FsReadOption_None, &bytesread);
			fsFileRead(&pd, SaveHeaderSize + playerSize + ItemCollectBitOffset + (0x262B / 8), CustomDesignPathPermit, sizeof(u8), FsReadOption_None, &bytesread);
			fsFileRead(&pd, SaveHeaderSize + playerSize + ItemCollectBitOffset + (0x2F99 / 8), CustomDesignProEditor, sizeof(u8), FsReadOption_None, &bytesread);
			fsFileRead(&pd, SaveHeaderSize + playerSize + ItemCollectBitOffset + (0x338B / 8), CustomDesignProEditorPlus, sizeof(u8), FsReadOption_None, &bytesread);
			fsFileRead(&pd, SaveHeaderSize + playerSize + ItemCollectBitOffset + (0x33C8 / 8), HalloweenCharacterColors, sizeof(u8), FsReadOption_None, &bytesread);
			fsFileRead(&pd, SaveHeaderSize + playerSize + ItemCollectBitOffset + (0x3963 / 8), Update200RecipeLicenseApp, sizeof(u8), FsReadOption_None, &bytesread);
			fsFileRead(&pd, SaveHeaderSize + playerSize + ItemCollectBitOffset + (0x39CC / 8), IslandLife101Service, sizeof(u8), FsReadOption_None, &bytesread);
			fsFileRead(&pd, SaveHeaderSize + playerSize + ItemCollectBitOffset + (0x39ED / 8), Top4FabHairstyles, sizeof(u8), FsReadOption_None, &bytesread);

			fsFileRead(&pd, SaveHeaderSize + playerSize + RecipesOffset, g_RecipeBook, g_RecipeBookSize, FsReadOption_None, &bytesread);

			dmntchtReadCheatProcessMemory(g_playerAddr + (i * playersOffset) + EventFlagsPlayerOffset + (669 * 2), &UpgradePocket30, sizeof(u16));
			dmntchtReadCheatProcessMemory(g_playerAddr + (i * playersOffset) + EventFlagsPlayerOffset + (670 * 2), &UpgradePocket40, sizeof(u16));

			fsFileRead(&pd, SaveHeaderSize + playerSize + ItemCollectBitOffset + (0x235C / 8), &ReceivedItemPocket30, sizeof(u8), FsReadOption_None, &bytesread);
			fsFileRead(&pd, SaveHeaderSize + playerSize + ItemCollectBitOffset + (0x2B84 / 8), &ReceivedItemPocket40, sizeof(u8), FsReadOption_None, &bytesread);

			//prep for 2.0 update
			u32 storageSizes[] = {
				0,
				80,
				120,
				240,
				320,
				400,
				800,
				1600,
				2400,
				3200,
				4000,
				5000
			};

			storageSize = storageSizes[houselvl + RcoStorageExpansion_v200_AddLevel];
			
			/*
			util::PrintToNXLink("Houselevel player %d: %d\n", i, houselvl);
			util::PrintToNXLink("Storage expansion 2.0.0: %d\n", RcoStorageExpansion_v200_AddLevel);
			util::PrintToNXLink("resulting storage size: %d\n", storageSize);
			*/
			
			u16 Update200Flags[] = {
				MainmenuRecipe_v2,						//Be a Chef! DIY Recipes+
				MainmenuMydesignPatternPlus,			//Custom Designs Patterns+
				UnlockInterior_CeilingFurniture,		//Pro Decorating License
				FenceRemakeEnable,						//Custom Fencing in a Flash
				MainmenuCamera1stPersonView,			//Pro Camera App
			};

			//0xXXXX modulo 8 is our bit offset in the byte(s).
			util::setBitBequalsA(HairStyleColor, sizeof(HairStyleColor) / sizeof(HairStyleColor[0]), HairStyles,						0x2359 % 8);
			util::setBitBequalsA(AddHairStyle4, StylishHairStyles,																		0x35C6 % 8);
			util::setBitBequalsA(ItemRingEnable, ToolRingItsEssential,																	0x2590 % 8);
			util::setBitBequalsA(GetLicenses, sizeof(GetLicenses) / sizeof(GetLicenses[0]), PermitsandLicenses,							0x2245 % 8);
			util::setBitBequalsA(GetLicenseGrdMydesign, CustomDesignPathPermit,															0x262B % 8);
			util::setBitBequalsA(UnlockMyDesignProCategory, CustomDesignProEditor,														0x2F99 % 8);
			util::setBitBequalsA(UnlockMydesignPro2, CustomDesignProEditorPlus,															0x338B % 8);
			util::setBitBequalsA(AddHalloweenColor, sizeof(AddHalloweenColor) / sizeof(AddHalloweenColor[0]), HalloweenCharacterColors,	0x33C8 % 8);
			util::setBitBequalsA(Update200Flags, sizeof(Update200Flags) / sizeof(Update200Flags[0]), Update200RecipeLicenseApp,			0x3963 % 8);
			util::setBitBequalsA(MainmenuTips, IslandLife101Service,																	0x39CC % 8);
			util::setBitBequalsA(UnlockPlayerHair37to47[0], Top4FabHairstyles,															0x39ED % 8); //if they got the first, they bought them lol

			//overlapping byte
			ReceivedItemPocket30 = HairStyles[0];
			if (UpgradePocket30 == 1) {
				pocket1Size += 0xA;
				ExpandBaggage = 0x01;
				ReceivedItemPocket30 |= (1 << 4);
				SellPocket40 = 1;
				if (UpgradePocket40 == 1) {
					pocket1Size += 0xA;
					ReceivedItemPocket40 |= (1 << 4);
					ExpandBaggage = 0x02;
				}
				else if (((ReceivedItemPocket40 >> 4) & 1) == 1) ReceivedItemPocket40 ^= (1 << 4);
			}
			else if (((ReceivedItemPocket30 >> 4) & 1) == 1) ReceivedItemPocket30 ^= (1 << 4);



			//Editing Recipes based on our flags

			for (u16 i : MainmenuRecipes) {
				util::SetFlag(g_RecipeBook, i, MainmenuRecipe);
			}
			for (u16 i : PrettyGoodToolsRecipesRecipes) {
				//util::SetFlag(g_RecipeBook, i, ((PrettyGoodToolsRecipes[0] >> (0x2405 % 8)) & 0x1));
				//fuck it, lets just give it to them
				util::SetFlag(g_RecipeBook, i, MainmenuRecipe);
				util::setBitBequalsA(MainmenuRecipe, PrettyGoodToolsRecipes, 0x2405 % 8);
			}
			for (u16 i : BeAChefRecipes) {
				util::SetFlag(g_RecipeBook, i, MainmenuRecipe_v2);
			}

			u16 g_SpecialityFruit = 0;
			dmntchtReadCheatProcessMemory(g_mainAddr + SaveFgOffset + SpecialityFruitOffset, &g_SpecialityFruit, sizeof(u16));
			//util::PrintToNXLink("SpecialityFruit: %04d\n", g_SpecialityFruit);
			//util::PrintToNXLink("Smoothie: 0x%04X\n", TownfruitSmoothiesMap.find(g_SpecialityFruit)->second);
			util::SetFlag(g_RecipeBook, TownfruitSmoothiesMap.find(g_SpecialityFruit)->second, MainmenuRecipe_v2);

			if (OwlGotDiyRecipe == 1 || P2_CreatedAfterOwlMoving == 1) {
				//Vaulting Pole is unlocked with MainmenuRecipe when its a player other than player 1
				//just gonna use MainmenuRecipe, because P1 obviously has it unlocked anyway
				util::SetFlag(g_RecipeBook, 0x0B7, MainmenuRecipe);
			}

			/*
			util::PrintToNXLink("Recipe Book:\n");
			for (int i = 0; i < 0x100; i++) {
				if (((i + 1) % 16) == 0) {
					util::PrintToNXLink("%02X\n", g_RecipeBook[i]);
				}
				else {
					util::PrintToNXLink("%02X ", g_RecipeBook[i]);
				}
			}

			util::PrintToNXLink("AddHairStyle1: %d\n",HairStyleColor[0]);
			util::PrintToNXLink("AddHairStyle2: %d\n", HairStyleColor[1]);
			util::PrintToNXLink("AddHairStyle3: %d\n", HairStyleColor[2]);
			util::PrintToNXLink("HairStyles: 0x%X\n", (HairStyles[0] >> 1) & 7);
			util::PrintToNXLink("AddHairStyle4: %d\n", AddHairStyle4);
			util::PrintToNXLink("StylishHairStyles: 0x%X\n", (StylishHairStyles[0] >> 6) & 1);

			util::PrintToNXLink("ItemRingEnable: %d\n", ItemRingEnable);
			util::PrintToNXLink("Tool Ring: It's Essential!: 0x%X\n", ToolRingItsEssential[0] & 1);

			util::PrintToNXLink("GetLicenseGrdStone: %d\n", GetLicenses[0]);
			util::PrintToNXLink("GetLicenseGrdBrick: %d\n", GetLicenses[1]);
			util::PrintToNXLink("GetLicenseGrdDarkSoil: %d\n", GetLicenses[2]);
			util::PrintToNXLink("GetLicenseGrdStonePattern: %d\n", GetLicenses[3]);
			util::PrintToNXLink("GetLicenseGrdSand: %d\n", GetLicenses[4]);
			util::PrintToNXLink("GetLicenseGrdTile: %d\n", GetLicenses[5]);
			util::PrintToNXLink("GetLicenseGrdWood: %d\n", GetLicenses[6]);
			util::PrintToNXLink("GetLicenseRiver: %d\n", GetLicenses[7]);
			util::PrintToNXLink("GetLicenseCliff: %d\n", GetLicenses[8]);

			u16 perms;
			memcpy(&perms, PermitsandLicenses, sizeof(perms));
			util::PrintToNXLink("PermitsandLicenses: 0x%X\n", (perms >> (0x2245 % 8)) & 0x1FF);

			util::PrintToNXLink("UnlockMyDesignProCategory: %d\n", UnlockMyDesignProCategory);
			util::PrintToNXLink("Custom Design Pro Editor: 0x%X\n", (CustomDesignProEditor[0] >> (0x2F99 % 8)) & 1);
			util::PrintToNXLink("UnlockMydesignPro2: %d\n", UnlockMydesignPro2);
			util::PrintToNXLink("Custom Design Pro Editor Plus: 0x%X\n", (CustomDesignProEditorPlus[0] >> (0x338B % 8)) & 1);

			util::PrintToNXLink("AddBodyColor: %d\n", AddHalloweenColor[0]);
			util::PrintToNXLink("AddEyeColor: %d\n", AddHalloweenColor[1]);
			util::PrintToNXLink("AddCheekColor: %d\n", AddHalloweenColor[2]);
			util::PrintToNXLink("Halloween Character Colors: 0x%X\n", (HalloweenCharacterColors[0] >> (0x33C8 % 8)) & 0x7);
			*/

#if DEBUG
			/*
			g_dumping_menu->LogAddLine("AddHairStyle1: " + std::to_string(HairStyleColor[0]));
			g_dumping_menu->LogAddLine("AddHairStyle2: " + std::to_string(HairStyleColor[1]));
			g_dumping_menu->LogAddLine("AddHairStyle3: " + std::to_string(HairStyleColor[2]));
			g_dumping_menu->LogAddLine("HairStyles: " + std::to_string((HairStyles[0] >> 1) & 7));
			g_dumping_menu->LogAddLine("AddHairStyle4: " + std::to_string(AddHairStyle4));
			g_dumping_menu->LogAddLine("StylishHairStyles: " + std::to_string((StylishHairStyles[0] >> 6) & 1));
			
			g_dumping_menu->LogAddLine("ItemRingEnable: " + std::to_string(ItemRingEnable));
			g_dumping_menu->LogAddLine("Tool Ring: It's Essential!: " + std::to_string(ToolRingItsEssential[0] & 1));
			

			g_dumping_menu->LogAddLine("GetLicenseGrdStone: " + std::to_string(GetLicenses[0]));
			g_dumping_menu->LogAddLine("GetLicenseGrdBrick: " + std::to_string(GetLicenses[1]));
			g_dumping_menu->LogAddLine("GetLicenseGrdDarkSoil: " + std::to_string(GetLicenses[2]));
			g_dumping_menu->LogAddLine("GetLicenseGrdStonePattern: " + std::to_string(GetLicenses[3]));
			g_dumping_menu->LogAddLine("GetLicenseGrdSand: " + std::to_string(GetLicenses[4]));
			g_dumping_menu->LogAddLine("GetLicenseGrdTile: " + std::to_string(GetLicenses[5]));
			g_dumping_menu->LogAddLine("GetLicenseGrdWood: " + std::to_string(GetLicenses[6]));
			g_dumping_menu->LogAddLine("GetLicenseRiver: " + std::to_string(GetLicenses[7]));
			g_dumping_menu->LogAddLine("GetLicenseCliff: " + std::to_string(GetLicenses[8]));
			memcpy(&perms, PermitsandLicenses, sizeof(perms));
			g_dumping_menu->LogAddLine("PermitsandLicenses: " + std::to_string(((perms >> (0x2245 % 8)) & 0x1FF)));

			g_dumping_menu->LogAddLine("GetLicenseGrdMydesign: " + std::to_string(GetLicenseGrdMydesign));
			g_dumping_menu->LogAddLine("CustomDesignPathPermit: " + std::to_string(((CustomDesignPathPermit[0] >> (0x262B % 8)) & 1)));
			*/

			g_dumping_menu->LogAddLine("UnlockMyDesignProCategory: " + std::to_string(UnlockMyDesignProCategory));
			g_dumping_menu->LogAddLine("Custom Design Pro Editor: " + std::to_string(((CustomDesignProEditor[0] >> (0x2F99 % 8)) & 1)));
			g_dumping_menu->LogAddLine("UnlockMydesignPro2: " + std::to_string(UnlockMydesignPro2));
			g_dumping_menu->LogAddLine("Custom Design Pro Editor Plus: " + std::to_string(((CustomDesignProEditorPlus[0] >> (0x338B % 8)) & 1)));

			g_dumping_menu->LogAddLine("AddBodyColor: " + std::to_string(AddHalloweenColor[0]));
			g_dumping_menu->LogAddLine("AddEyeColor: " + std::to_string(AddHalloweenColor[1]));
			g_dumping_menu->LogAddLine("AddCheekColor: " + std::to_string(AddHalloweenColor[2]));
			g_dumping_menu->LogAddLine("Halloween Character Colors: " + std::to_string(((HalloweenCharacterColors[0] >> (0x33C8 % 8)) & 0x7)));

			g_dumping_menu->LogAddLine("SellPocket40: " + std::to_string(SellPocket40));
			g_dumping_menu->LogAddLine("UpgradePocket30: " + std::to_string(UpgradePocket30));
			g_dumping_menu->LogAddLine("UpgradePocket40: " + std::to_string(UpgradePocket40));
			g_dumping_menu->LogAddLine("Pocket Organization Guide: " + std::to_string(((ReceivedItemPocket30 >> 4) & 1)));
			g_dumping_menu->LogAddLine("Ultimate Pocket Stuffing Guide: " + std::to_string(((ReceivedItemPocket40 >> 4) & 1)));
			g_dumping_menu->LogAddLine("ExpandBaggage: " + std::to_string(ExpandBaggage));
			g_dumping_menu->LogAddLine("Houselvl: " + std::to_string(houselvl));
			g_dumping_menu->LogAddLine("Storage expansion 2.0.0: " + std::to_string(RcoStorageExpansion_v200_AddLevel));
			g_dumping_menu->LogAddLine("StorageSize: " + std::to_string(storageSize));
#endif

			fsFileWrite(&pd, SaveHeaderSize + playerSize + ItemCollectBitOffset + (0x2359 / 8), HairStyles, sizeof(u8), FsWriteOption_Flush);
			fsFileWrite(&pd, SaveHeaderSize + playerSize + ItemCollectBitOffset + (0x35C6 / 8), StylishHairStyles, sizeof(u8), FsWriteOption_Flush);
			fsFileWrite(&pd, SaveHeaderSize + playerSize + ItemCollectBitOffset + (0x2405 / 8), PrettyGoodToolsRecipes, sizeof(u8), FsWriteOption_Flush);
			fsFileWrite(&pd, SaveHeaderSize + playerSize + ItemCollectBitOffset + (0x262B / 8), CustomDesignPathPermit, sizeof(u8), FsWriteOption_Flush);
			fsFileWrite(&pd, SaveHeaderSize + playerSize + ItemCollectBitOffset + (0x2590 / 8), ToolRingItsEssential, sizeof(u8), FsWriteOption_Flush);
			fsFileWrite(&pd, SaveHeaderSize + playerSize + ItemCollectBitOffset + (0x2245 / 8), PermitsandLicenses, sizeof(u16), FsWriteOption_Flush);
			fsFileWrite(&pd, SaveHeaderSize + playerSize + ItemCollectBitOffset + (0x2F99 / 8), CustomDesignProEditor, sizeof(u8), FsWriteOption_Flush);
			fsFileWrite(&pd, SaveHeaderSize + playerSize + ItemCollectBitOffset + (0x338B / 8), CustomDesignProEditorPlus, sizeof(u8), FsWriteOption_Flush);
			fsFileWrite(&pd, SaveHeaderSize + playerSize + ItemCollectBitOffset + (0x33C8 / 8), HalloweenCharacterColors, sizeof(u8), FsWriteOption_Flush);

			fsFileWrite(&pd, SaveHeaderSize + playerSize + ItemCollectBitOffset + (0x3963 / 8), Update200RecipeLicenseApp, sizeof(u8), FsWriteOption_Flush);
			fsFileWrite(&pd, SaveHeaderSize + playerSize + ItemCollectBitOffset + (0x39CC / 8), IslandLife101Service, sizeof(u8), FsWriteOption_Flush);
			fsFileWrite(&pd, SaveHeaderSize + playerSize + ItemCollectBitOffset + (0x39ED / 8), Top4FabHairstyles, sizeof(u8), FsWriteOption_Flush);

			fsFileWrite(&pd, SaveHeaderSize + playerSize + RecipesOffset, g_RecipeBook, g_RecipeBookSize, FsWriteOption_Flush);

			fsFileWrite(&pd, StorageSizeOffset, &storageSize, sizeof(u32), FsWriteOption_Flush);
			fsFileWrite(&pd, Pocket1SizeOffset, &pocket1Size, sizeof(u32), FsWriteOption_Flush);
			fsFileWrite(&pd, ExpandBaggageOffset, &ExpandBaggage, sizeof(u8), FsWriteOption_Flush);
			fsFileWrite(&pd, SaveHeaderSize + EventFlagsPlayerOffset + (672 * 2), &SellPocket40, sizeof(u16), FsWriteOption_Flush);
			fsFileWrite(&pd, SaveHeaderSize + playerSize + ItemCollectBitOffset + (0x235C / 8), &ReceivedItemPocket30, sizeof(u8), FsWriteOption_Flush);
			fsFileWrite(&pd, SaveHeaderSize + playerSize + ItemCollectBitOffset + (0x2B84 / 8), &ReceivedItemPocket40, sizeof(u8), FsWriteOption_Flush);

			delete g_RecipeBook;
			g_dumping_menu->LogAddLine("Applying fixes to player " + std::to_string(i + 1) + ": successful", applyingFixesLogEntryIndex);
			fsFileClose(&pd);
		}
	}

	void Fixes() {
		FixMain();
		addProgress(0.05f);
		FixPlayers();
		addProgress(0.05f);

#if !DEBUG_UI
		*g_dumping_state = dbk::DumpingMenu::DumpState::NeedsSave;
#else
		enableButton();
#endif
	}

	void Save() {
		g_dumping_menu->LogAddLine("Hashing and encrypting save files...");
		//grabbing tick once, because else the Random::init() function would run for each file and i tink they all need to have the same tick?
		u32 currenttick = static_cast<u32>(svcGetSystemTick());
		float progress = g_progress_percent_last_function;
		//this shouldnt be an issue, since the directory lists only get fetched once at the start
		handleEncryption(g_pathOut, g_pathOut, currenttick, g_datcount);
		setProgress(progress + 0.40f);
		g_dumping_menu->LogEditLastElement("Hashing and encrypting save files: successful");
		fsFsClose(&g_fsSdmc);
		delete g_AccountTableBuffer;
		delete g_buffer;
		enableButton();
		*g_dumping_state = dbk::DumpingMenu::DumpState::End;
	}

}