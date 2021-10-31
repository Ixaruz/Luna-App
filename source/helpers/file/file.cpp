#include <cstdio>
#include <algorithm>
#include <cstring>
#include <vector>
#include <switch.h>
#include <dirent.h>
#include <unistd.h>
#include <cstdarg>
#include <sys/stat.h>
#include "file.h"
#include "../../util.h"

typedef struct
{
    uint64_t appID;
    uint8_t saveType;
    uint8_t saveRank;
    uint16_t saveIndex;
    uint64_t saveSize;
    uint64_t availableSize;
    uint64_t journalSize;
} svInfo;

static std::string wd;

static std::vector<std::string> pathFilter;

static char pathbuff[FS_MAX_PATH];

Result fsres;

static struct
{
    bool operator()(const fs::dirItem& a, const fs::dirItem& b)
    {
        if(a.isDir() != b.isDir())
            return a.isDir();

        for(unsigned i = 0; i < a.getItm().length(); i++)
        {
            char charA = tolower(a.getItm()[i]);
            char charB = tolower(b.getItm()[i]);
            if(charA != charB)
                return charA < charB;
        }
        return false;
    }
} sortDirList;


fs::dirItem::dirItem(const std::string& pathTo, const std::string& sItem)
{
    itm = sItem;

    std::string fullPath = pathTo + sItem;
    struct stat s;
    if(stat(fullPath.c_str(), &s) == 0 && S_ISDIR(s.st_mode))
        dir = true;
}

std::string fs::dirItem::getName() const
{
    size_t extPos = itm.find_last_of('.'), slPos = itm.find_last_of('/');
    if(extPos == itm.npos)
        return "";

    return itm.substr(slPos + 1, extPos);
}

std::string fs::dirItem::getExt() const
{
    size_t extPos = itm.find_last_of('.');
    if(extPos == itm.npos)
        return "";//Folder or no extension
    return itm.substr(extPos + 1, itm.npos);
}

fs::dirList::dirList(const std::string& _path)
{
    path = _path;
    d = opendir(path.c_str());

    while((ent = readdir(d)))
        item.emplace_back(path, ent->d_name);

    closedir(d);

    std::sort(item.begin(), item.end(), sortDirList);
}

void fs::dirList::reassign(const std::string& _path)
{
    path = _path;

    d = opendir(path.c_str());

    item.clear();

    while((ent = readdir(d)))
        item.emplace_back(path, ent->d_name);

    closedir(d);

    std::sort(item.begin(), item.end(), sortDirList);
}

void fs::dirList::rescan()
{
    item.clear();
    d = opendir(path.c_str());

    while((ent = readdir(d)))
        item.emplace_back(path, ent->d_name);

    closedir(d);

    std::sort(item.begin(), item.end(), sortDirList);
}

fs::dataFile::dataFile(const std::string& _path)
{
    f = fopen(_path.c_str(), "r");
    if(f != NULL)
        opened = true;
}

fs::dataFile::~dataFile()
{
    fclose(f);
}

bool fs::dataFile::readNextLine(bool proc)
{
    bool ret = false;
    char tmp[1024];
    while(fgets(tmp, 1024, f))
    {
        if(tmp[0] != '#' && tmp[0] != '\n' && tmp[0] != '\r')
        {
            line = tmp;
            ret = true;
            break;
        }
    }
    util::stripChar('\n', line);
    util::stripChar('\r', line);
    if(proc)
        procLine();

    return ret;
}

void fs::dataFile::procLine()
{
    size_t pPos = line.find_first_of('('), ePos = line.find_first_of('=');
    if(pPos != line.npos || ePos != line.npos)
    {
        lPos = ePos < pPos ? ePos : pPos;
        name.assign(line.begin(), line.begin() + lPos);
    }
    else
        name = "NULL";
    util::stripChar(' ', name);
    ++lPos;
}

std::string fs::dataFile::getNextValueStr()
{
    std::string ret = "";
    //Skip all spaces until we hit actual text
    size_t pos1 = line.find_first_not_of(", ", lPos);
    //If reading from quotes
    if(line[pos1] == '"')
        lPos = line.find_first_of('"', ++pos1);
    else
        lPos = line.find_first_of(",;\n", pos1);//Set lPos to end of string we want. This should just set lPos to the end of the line if it fails, which is ok

    return line.substr(pos1, lPos++ - pos1);
}

int fs::dataFile::getNextValueInt()
{
    int ret = 0;
    std::string no = getNextValueStr();
    if(no[0] == '0' && tolower(no[1]) == 'x')
        ret = strtoul(no.c_str(), NULL, 16);
    else
        ret = strtoul(no.c_str(), NULL, 10);

    return ret;
}

void fs::copyFile(FsFileSystem *fs, const std::string& from, const std::string& to)
{
    uint8_t *buff = new uint8_t[BUFF_SIZE];


    FsFile in;
    FsFile out;
    //from file
    std::snprintf(pathbuff, FS_MAX_PATH, from.c_str());
#if DEBUG_FS
    fsres = fsFsOpenFile(fs, pathbuff, FsOpenMode_Read, &in);
    char txt[sizeof(Result)];
    snprintf(txt, sizeof(Result), "%u", fsres);
    (*logelm)->addLine(txt);
    std::this_thread::sleep_for(std::chrono::seconds(1));
#else
    fsFsOpenFile(fs, pathbuff, FsOpenMode_Read, &in);
#endif

    u64 readIn = BUFF_SIZE;
    u64 off = 0;
    s64 fsize = 0;
    u64 bytesread = 0;

    fsFileGetSize(&in, &fsize);
    //clear our path buffer or bad things will happen
    memset(pathbuff, 0, FS_MAX_PATH);
    //to file
    std::snprintf(pathbuff, FS_MAX_PATH, to.c_str());
    fsFsDeleteFile(fs, pathbuff);
    fsFsCreateFile(fs, pathbuff, fsize, 0);
#if DEBUG_FS
    fsres = fsFsOpenFile(fs, pathbuff, FsOpenMode_Write, &out);
    snprintf(txt, sizeof(Result), "%u", fsres);
    (*logelm)->addLine(txt);
    std::this_thread::sleep_for(std::chrono::seconds(1));
#else
    fsFsOpenFile(fs, pathbuff, FsOpenMode_Write, &out);
#endif

    for (off = 0; off < (u64)fsize; off += readIn) {
        if (readIn > (u64)fsize - off)
            readIn = (u64)fsize - off;

        fsFileRead(&in, off, buff, readIn, FsReadOption_None, &bytesread);
        fsFileWrite(&out, off, buff, readIn, FsWriteOption_Flush);

    }

    //getting file name
    std::size_t dirPos = to.find_last_of("/");
    std::string file = to.substr(dirPos);

    fsFileClose(&in);
    fsFileClose(&out);
    
    delete[] buff;
}

void fs::copyDirToDir(FsFileSystem *fs, const std::string& from, const std::string& to)
{
    dirList list(from);

    for(unsigned i = 0; i < list.getCount(); i++)
    {
        if(pathIsFiltered(from + list.getItem(i)))
            continue;

        if(list.isDir(i))
        {
            std::string newFrom = from + list.getItem(i) + "/";
            std::string newTo   = to + list.getItem(i) + "/";
            mkdir(newTo.substr(0, newTo.length() - 1).c_str(), 0777);

            copyDirToDir(fs, newFrom, newTo);
        }
        else
        {
            std::string fullFrom = from + list.getItem(i);
            std::string fullTo   = to   + list.getItem(i);
            copyFile(fs, fullFrom, fullTo);
        }
    }
}


bool fs::readSvi(const std::string& _path, FsSaveDataAttribute *attr, FsSaveDataCreationInfo *crInfo)
{
    FILE *sviIn = fopen(_path.c_str(), "rb");
    if(!sviIn)
        return false;

    svInfo infoIn;
    fread(&infoIn, sizeof(svInfo), 1, sviIn);
    fclose(sviIn);

    attr->application_id = infoIn.appID;
    attr->save_data_type = infoIn.saveType;
    attr->save_data_rank = infoIn.saveRank;
    attr->save_data_index = infoIn.saveIndex;

    crInfo->owner_id = infoIn.appID;
    crInfo->save_data_size = infoIn.saveSize;
    crInfo->available_size = infoIn.availableSize;
    crInfo->journal_size = infoIn.journalSize;
    crInfo->save_data_space_id = FsSaveDataSpaceId_User;

    return true;
}

Result fs::createSaveDataFileSystem(const FsSaveDataAttribute *attr, const FsSaveDataCreationInfo *crInfo)
{
    Service *fs = fsGetServiceSession();
    struct
    {
        FsSaveDataAttribute attr;
        FsSaveDataCreationInfo create;
        uint32_t unk0;
        uint8_t unk1[0x06];
    } in = {*attr, *crInfo, 0, {0}};

    if(attr->save_data_type != FsSaveDataType_Device)
    {
        in.unk0 = 0x40060;
        in.unk1[0] = 1;
    }

    return serviceDispatchIn(fs, 22, in);
}

void fs::delfile(const std::string& path)
{
    fsremove(path.c_str());
}

void fs::delDir(const std::string& path)
{
    dirList list(path);
    for(unsigned i = 0; i < list.getCount(); i++)
    {
        if(pathIsFiltered(path + list.getItem(i)))
            continue;

        if(list.isDir(i))
        {
            std::string newPath = path + list.getItem(i) + "/";
            delDir(newPath);

            std::string delPath = path + list.getItem(i);
            rmdir(delPath.c_str());
        }
        else
        {
            std::string delPath = path + list.getItem(i);
            std::remove(delPath.c_str());
        }
    }
    rmdir(path.c_str());
}

void fs::addPathFilter(const std::string& _path)
{
    pathFilter.push_back(_path);
}

void fs::loadPathFilters(const std::string& _file)
{
    if(fs::fileExists(_file))
    {
        fs::dataFile filter(_file);
        while(filter.readNextLine(false))
            pathFilter.push_back(filter.getLine());
    }
}

bool fs::pathIsFiltered(const std::string& _path)
{
    if(pathFilter.empty())
        return false;

    for(std::string& _p : pathFilter)
    {
        if(_path == _p)
            return true;
    }

    return false;
}

void fs::freePathFilters()
{
    pathFilter.clear();
}

std::string fs::getFileProps(const std::string& _path)
{
    std::string ret = "";

    FILE *get = fopen(_path.c_str(), "rb");
    if(get != NULL)
    {
        //Size
        fseek(get, 0, SEEK_END);
        unsigned fileSize = ftell(get);
        fseek(get, 0, SEEK_SET);

        fclose(get);

        //Probably add more later

        char tmp[256];
        std::sprintf(tmp, "Path: #%s#\nSize: %u", _path.c_str(), fileSize);

        ret = tmp;
    }
    return ret;
}

void fs::getDirProps(const std::string& _path, uint32_t& dirCount, uint32_t& fileCount, uint64_t& totalSize)
{
    fs::dirList list(_path);

    for(unsigned i = 0; i < list.getCount(); i++)
    {
        if(list.isDir(i))
        {
            ++dirCount;
            std::string newPath = _path + list.getItem(i) + "/";
            uint32_t dirAdd = 0, fileAdd = 0;
            uint64_t sizeAdd = 0;

            getDirProps(newPath, dirAdd, fileAdd, sizeAdd);
            dirCount += dirAdd;
            fileCount += fileAdd;
            totalSize += sizeAdd;
        }
        else
        {
            ++fileCount;
            std::string filePath = _path + list.getItem(i);

            FILE *gSize = fopen(filePath.c_str(), "rb");
            fseek(gSize, 0, SEEK_END);
            size_t fSize = ftell(gSize);
            fclose(gSize);

            totalSize += fSize;
        }
    }
}

bool fs::fileExists(const std::string& path)
{
    FILE *test = fopen(path.c_str(), "rb");
    if(test != NULL)
    {
        fclose(test);
        return true;
    }

    return false;
}

size_t fs::fsize(const std::string& _f)
{
    size_t ret = 0;
    FILE *get = fopen(_f.c_str(), "rb");
    if(get != NULL)
    {
        fseek(get, 0, SEEK_END);
        ret = ftell(get);
    }
    fclose(get);
    return ret;
}

bool fs::isDir(const std::string& _path)
{
    struct stat s;
    return stat(_path.c_str(), &s) == 0 && S_ISDIR(s.st_mode);
}

