#ifndef FILE_H
#define FILE_H
#pragma once

#include <string>
#include <cstdio>
#include <vector>
#include <switch.h>
#include <dirent.h>
#include <minizip/zip.h>
#include <minizip/unzip.h>

#include "../../luna.h"

#include "fsfile.h"

//2MB of buffer
#define BUFF_SIZE 0x200000


namespace fs
{
    void copyFile(FsFileSystem *fs, const std::string &from, const std::string &to);

    //Recursively copies 'from' to 'to'
    void copyDirToDir(FsFileSystem *fs, const std::string &from, const std::string &to);


    //Reads svi from path and writes needed info to attr and crInfo. Does NOT set account id for attr
    bool readSvi(const std::string& _path, FsSaveDataAttribute *attr, FsSaveDataCreationInfo *crInfo);

    //Creates save data filesystem on NAND
    Result createSaveDataFileSystem(const FsSaveDataAttribute *attr, const FsSaveDataCreationInfo *crInfo);

    //deletes file
    void delfile(const std::string& path);
    //Recursively deletes 'path'
    void delDir(const std::string& path);

    //Loads paths to filter from backup/deletion
    void addPathFilter(const std::string& _path);
    void loadPathFilters(const std::string& _file);
    bool pathIsFiltered(const std::string& _path);
    void freePathFilters();

    //returns file properties as C++ string
    std::string getFileProps(const std::string& _path);

    //Recursively retrieves info about dir _path
    void getDirProps(const std::string& _path, uint32_t& dirCount, uint32_t& fileCount, uint64_t& totalSize);

    bool fileExists(const std::string& _path);
    //Returns file size
    size_t fsize(const std::string& _f);
    bool isDir(const std::string& _path);


    class dirItem
    {
        public:
            dirItem(const std::string& pathTo, const std::string& sItem);
            std::string getItm() const { return itm; }
            std::string getName() const;
            std::string getExt() const;
            bool isDir() const { return dir; }

        private:
            std::string itm;
            bool dir = false;
    };

    //Just retrieves a listing for _path and stores it in item vector
    class dirList
    {
        public:
            dirList() = default;
            dirList(const std::string& _path);
            void reassign(const std::string& _path);
            void rescan();

            std::string getItem(int index) const { return item[index].getItm(); }
            std::string getItemName(int index) const { return item[index].getName(); }
            std::string getItemExt(int index) const { return item[index].getExt(); }
            bool isDir(int index) const { return item[index].isDir(); }
            unsigned getCount() const { return item.size(); }

        private:
            DIR *d;
            struct dirent *ent;
            std::string path;
            std::vector<dirItem> item;
    };

    class dataFile
    {
        public:
            dataFile(const std::string& _path);
            ~dataFile();

            bool isOpen() const { return opened; }

            bool readNextLine(bool proc);
            //Finds where variable name ends. When a '(' or '=' is hit. Strips spaces
            void procLine();
            std::string getLine() const { return line; }
            std::string getName() const { return name; }
            //Reads until ';', ',', or '\n' is hit and returns as string.
            std::string getNextValueStr();
            int getNextValueInt();

        private:
            FILE *f;
            std::string line, name;
            size_t lPos = 0;
            bool opened = false;
    };

}

#endif // FILE_H
