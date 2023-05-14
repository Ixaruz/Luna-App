#pragma once
#include <switch.h>
#include "helpers/file/file.h"
#include "util.h"

typedef struct {
    /* 0x00 */ u32 Major;
    /* 0x04 */ u32 Minor;
    /* 0x08 */ u16 Unk1;
    /* 0x0A */ u16 HeaderRevision;
    /* 0x0C */ u16 Unk2;
    /* 0x0E */ u16 SaveRevision;
} FileHeaderInfo;

struct TemplateCheckResult {
    Error error;
    std::string additional_info;
};

//define all valid save RevisionInfo here
const std::vector<FileHeaderInfo*> RevisionInfo = std::vector<FileHeaderInfo*>{
    new FileHeaderInfo { /*Major*/ 0x80009, /*Minor*/ 0x80085, /*Unk1*/ 2, /*HeaderRevision*/ 0, /*Unk2*/ 2, /*SaveRevision*/ 22}, // 2.0.0
    new FileHeaderInfo { /*Major*/ 0x80009, /*Minor*/ 0x80085, /*Unk1*/ 2, /*HeaderRevision*/ 0, /*Unk2*/ 2, /*SaveRevision*/ 23}, // 2.0.1
    new FileHeaderInfo { /*Major*/ 0x80009, /*Minor*/ 0x80085, /*Unk1*/ 2, /*HeaderRevision*/ 0, /*Unk2*/ 2, /*SaveRevision*/ 24}, // 2.0.2
    new FileHeaderInfo { /*Major*/ 0x80009, /*Minor*/ 0x80085, /*Unk1*/ 2, /*HeaderRevision*/ 0, /*Unk2*/ 2, /*SaveRevision*/ 25}, // 2.0.3
    new FileHeaderInfo { /*Major*/ 0x80009, /*Minor*/ 0x80085, /*Unk1*/ 2, /*HeaderRevision*/ 0, /*Unk2*/ 2, /*SaveRevision*/ 26}, // 2.0.4
    new FileHeaderInfo { /*Major*/ 0x80009, /*Minor*/ 0x80085, /*Unk1*/ 2, /*HeaderRevision*/ 0, /*Unk2*/ 2, /*SaveRevision*/ 27}, // 2.0.5
    new FileHeaderInfo { /*Major*/ 0x80009, /*Minor*/ 0x80085, /*Unk1*/ 2, /*HeaderRevision*/ 0, /*Unk2*/ 2, /*SaveRevision*/ 28}, // 2.0.6
};

class TemplateCheck {
private:
    bool players[8] = { false };
    bool personalfound[8] = { false };
    TemplateCheckResult result = { Error::Success, "" };

    FsFileSystem fsSdmc;

    void checkPlayers(u64 mainAddr) {
        if (mainAddr != 0x00) {
            static std::string playernumbers = "";
            for (u8 i = 0; i < 8; i++) {
                u128 AccountUID = 0;
                dmntchtReadCheatProcessMemory(mainAddr + GSavePlayerVillagerAccountOffset + i * GSavePlayerVillagerAccountSize, &AccountUID, 0x10);
                if (AccountUID != 0) {
                    players[i] = true;
                    if (access(std::string(std::string(LUNA_TEMPLATE_DIR) + "Villager" + std::to_string(i)).c_str(), F_OK) == -1) {
                        result.error = Error::TemplateNotEnoughPlayers;
                        playernumbers.append((playernumbers.empty() ? "" : ", ") + std::to_string(i));
                    }
                }
            }
            result.additional_info = playernumbers;
        }
    }
public:
    TemplateCheck() {
        fsOpenSdCardFileSystem(&fsSdmc);
    }

    ~TemplateCheck(){
        fsFsClose(&fsSdmc);
    }

    TemplateCheckResult CheckTemplateFiles(const std::string& path, u64 mainAddr, bool issubdir = false) {

        if (!issubdir) {
            checkPlayers(mainAddr);
        }

        u8 maskeditemscount = 0;

        FsFile check;
        u64 bytesread = 0;

        static FileHeaderInfo checkDAT = { 0 };

        bool mainfound = false;
        bool correctRevision = false;

        fs::dirList list(path);
        u32 listcount = list.getCount();

        if (listcount == 0) {
            result.error = Error::TemplateMissing;
            return result;
        }

        for (u32 i = 0; i < listcount; i++) {
            //skip over landname.dat
            if (list.getItem(i) == "landname.dat") {
                maskeditemscount++;
                continue;
            }
            //either a misc file or a folder that we have to check inside
            if (list.getItemExt(i) != "dat") {
                maskeditemscount++;
                if (list.isDir(i)) {
                    std::string tobechecked = path + list.getItem(i) + "/";
                    result = CheckTemplateFiles(tobechecked, mainAddr, true);
                    if (result.error != Error::Success) {
                        return result;
                    }
                }
            }
            //.dat files
            else {
                std::string tobechecked = path + list.getItem(i);
                //check the Header files only
                if ((list.getItem(i).find("Header") != std::string::npos)) {
                    char pathbuffer[FS_MAX_PATH] = { 0 };
                    std::snprintf(pathbuffer, FS_MAX_PATH, tobechecked.c_str());
                    fsFsOpenFile(&fsSdmc, pathbuffer, FsOpenMode_Read, &check);
                    fsFileRead(&check, 0, &checkDAT, sizeof(FileHeaderInfo), FsReadOption_None, &bytesread);
                    for (auto& r : RevisionInfo) {
                        if (memcmp(r, &checkDAT, sizeof(FileHeaderInfo)) == 0) correctRevision = true;
                    }
                    if (!correctRevision) {
                        result.error = Error::TemplateWrongRevision;
                        char revision[50];
                        std::snprintf(revision, 50, "M = 0x%X, m = 0x%X, rev = %2u", checkDAT.Major, checkDAT.Minor, checkDAT.SaveRevision);
                        result.additional_info = std::string(revision);
                        return result;
                    }
                }
                if (util::getFilename(tobechecked) == "main.dat") mainfound = true;
                if (issubdir) {
                    size_t slPos = tobechecked.find_last_of('/');
                    u8 playernumber = std::stoi(tobechecked.substr(slPos - 1, 1));
                    if (util::getFilename(tobechecked) == "personal.dat") {
                        personalfound[playernumber] = true;
                        util::PrintToNXLink("Template Villager%d found\n", playernumber);
                    }
                }
            }
        }

        if (!issubdir) {
            if (!mainfound) {
                util::PrintToNXLink("missing main.dat\n");
                result.error = Error::TemplateMissingFiles;
            }

            if (result.error != Error::TemplateNotEnoughPlayers) {
                for (u8 i = 0; i < 8; i++) {
                    if (players[i] && !personalfound[i]) {
                        util::PrintToNXLink("missing personal.dat\n");
                        result.error = Error::TemplateMissingFiles;
                    }
                }
            }
        }

        if (((listcount - maskeditemscount) % 2) == 1) {
            util::PrintToNXLink("missing headerfiles (uneven number of files)\nDirectory: " + path + "\n");
            result.error = Error::TemplateMissingFiles;
        }

        return result;
    }

};