#pragma once
#include <switch.h>
#include "helpers\file\file.h"
#include "util.h"

typedef struct {
    /* 0x00 */ u32 Major;
    /* 0x04 */ u32 Minor;
    /* 0x08 */ u16 Unk1;
    /* 0x0A */ u16 HeaderRevision;
    /* 0x0C */ u16 Unk2;
    /* 0x0E */ u16 SaveRevision;
} FileHeaderInfo;

enum class CheckResult {
    Success,
    NoTemplate,
    MissingFiles,
    WrongRevision,
    NotEnoughPlayers,
};

struct Check {
    CheckResult check_result;
    std::string additional_info;
};

namespace temp {
    std::string getFilename(std::string& path) {
        size_t extPos = path.find_last_of('.'), slPos = path.find_last_of('/');
        if (extPos == path.npos)
            return "";

        return path.substr(slPos + 1, extPos);
    }
}

static bool players[8] = { false };
static bool personalfound[8] = { false };

Check CheckTemplateFiles(const std::string& path, u64 mainAddr, bool issubdir = false) {
    FsFileSystem fsSdmc;
    FsFile check;
    u64 bytesread = 0;
    fsOpenSdCardFileSystem(&fsSdmc);
    static FileHeaderInfo checkDAT = { 0 };
    Check chkres;
    chkres.check_result = CheckResult::Success;
    bool mainfound = false;
    u8 maskeditemscount = 0;

    bool exemptfrommemread = false;

    fs::dirList list(path);
    u32 listcount = list.getCount();

    if (listcount == 0) {
        chkres.check_result = CheckResult::NoTemplate;
        return chkres;
    }

    if (mainAddr == 0x60) {
        exemptfrommemread = true;
    }

    for (u32 i = 0; i < listcount; i++) {
        //skip over landname.dat
        if (list.getItem(i) == "landname.dat") {
            maskeditemscount++;
            continue;
        }

        if (list.isDir(i)) {
            maskeditemscount++;
            std::string tobechecked = path + list.getItem(i) + "/";
            chkres = CheckTemplateFiles(tobechecked, mainAddr, true);
            if (chkres.check_result != CheckResult::Success) {
                return chkres;
            }
        }

        else {
            std::string tobechecked = path + list.getItem(i);
            //check the Header files only
            if ((list.getItem(i).find("Header") != std::string::npos)) {
                char pathbuffer[FS_MAX_PATH] = { 0 };
                std::snprintf(pathbuffer, FS_MAX_PATH, tobechecked.c_str());
                fsFsOpenFile(&fsSdmc, pathbuffer, FsOpenMode_Read, &check);
                fsFileRead(&check, 0, &checkDAT, 0x10, FsReadOption_None, &bytesread);
                if (!(checkDAT.SaveRevision <= REVISION_SAVE && checkDAT.SaveRevision >= REVISION_SAVE_LAST) || checkDAT.Major != REVISION_MAJOR || checkDAT.Minor != REVISION_MINOR) {
                    chkres.check_result = CheckResult::WrongRevision;
                    char revision[50];
                    std::snprintf(revision, 50, "M = 0x%X, m = 0x%X, rev = %2u", checkDAT.Major, checkDAT.Minor, checkDAT.SaveRevision);
                    chkres.additional_info = std::string(revision);
                    return chkres;
                }
            }
            if (temp::getFilename(tobechecked) == "main.dat") mainfound = true;
            if (issubdir) {
                size_t slPos = tobechecked.find_last_of('/');
                u8 playernumber = std::stoi(tobechecked.substr(slPos - 1, 1));
                if (temp::getFilename(tobechecked) == "personal") personalfound[playernumber] = true;
            }
        }
    }

    static std::string playernumbers = "";
    if (!exemptfrommemread) {
        for (u8 i = 0; i < 8; i++) {
            u128 AccountUID = 0;
            dmntchtReadCheatProcessMemory(mainAddr + GSavePlayerVillagerAccountOffset + i * GSavePlayerVillagerAccountSize, &AccountUID, 0x10);
            if (AccountUID != 0) {
                if (access(std::string(std::string(LUNA_TEMPLATE_DIR) + "Villager" + std::to_string(i)).c_str(), F_OK) == -1) {
                    chkres.check_result = CheckResult::NotEnoughPlayers;
                    playernumbers.append((playernumbers.empty() ? "" : ", ") + std::to_string(i));
                    players[i] = true;
                }
            }
        }
    }
    chkres.additional_info = playernumbers;

    if (!mainfound && !issubdir) {
        printf("missing main.dat\n");
        chkres.check_result = CheckResult::MissingFiles;
    }
    for (u8 i = 0; i < 8; i++) {
        if (players[i] != personalfound[i]) {
            printf("missing personal.dat\n");
            chkres.check_result = CheckResult::MissingFiles;
        }
    }
    if (((listcount - maskeditemscount) % 2) == 1) {
        printf("missing headerfiles (uneven number of files)\n");
        chkres.check_result = CheckResult::MissingFiles;
    }

    return chkres;
}