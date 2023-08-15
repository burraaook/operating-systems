
#ifndef FAT12_H_
#define FAT12_H_

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <ctime>
#include <cstring>
#include <sstream>
#include "util.h"

namespace filesystem12
{

    struct superblock
    {
        int magicNumber;
        size_t rootPosition;
        size_t blockSize;
        size_t numberOfBlocks;
        size_t blockCount;
        size_t freeBlockCount;
        size_t fileCount;
        size_t directoryCount;
    };

    struct block
    {
        size_t blockNumber;
        ssize_t nextBlock;
        bool isDirectory;
        std::vector<char> data;
    };

    struct fat12_b
    {
        superblock sb;
        std::vector<block> blocks;
        std::vector<block> freeBlocks;
    };

    struct directoryEntry
    {
        std::string fileName;
        bool isDirectory;
        size_t length;
        std::string extension;
        std::string time;
        std::string date;
        size_t blockNumber;
    };
    struct directory
    {
        std::string directoryPath;
        size_t numberOfEntries;
        size_t blockNumber;
        std::vector<directoryEntry> entries;
    };

    struct file
    {
        std::string fileName;
        std::string fileExtension;
        std::string fileTime;
        std::string fileDate;
        size_t fileLength;
        size_t blockNumber;
    };

    class fatTable
    {

    private:
        fat12_b fat12;
        bool isInitialized;
        std::string fileSystemName;

        // helper functions
        bool serializeDirectory (const directory& d, std::vector<char>& data);
        bool deserializeDirectory (directory& d, const block& directoryBlock);
        bool readFat12 (fat12_b& fat12, std::ifstream& file);
        bool deserializeFile (file& f, const block& fileBlock);
        bool serializeFile (file &f, const std::string& fileNameToRead, std::vector<char>& data);
        bool writeFileContent (const block& fileBlock, std::string fileNameToWrite, size_t fileLength);

        // starts from root till the wanted directory
        bool searchDirectory (directory d, std::string path, directory& result, const std::string& directoryName);

        bool updateDirectory (directory& d);

        bool findBlock (size_t blockNumber, block& b);
        bool setBlock (size_t blockNumber, const block& b);

        // remove file
        bool removeFile (block& startBlock);
        bool removeBlock (block& b);
        
    public:
        
        // constructor
        fatTable ();

        // destructor
        ~fatTable ();

        // start the file system, with reading block size from binary file
        bool start (std::string fileName);

        // initalize the file system, blocksize can be 0.5, 1, 2, 4
        bool init (std::string blockSize);

        // write to binary file
        bool write (std::string fileName);

        // read from binary file
        bool read (std::string fileName);

        // other operations

        // list the contents of the directory
        bool opDir (std::string path);

        // create a new directory
        bool opMkdir (std::string path);

        // remove a directory
        bool opRmdir (std::string path);

        // give information about file system
        bool opDumpe2fs ();

        // creates and writes data to the file
        bool opWrite (std::string path, std::string fileNameToReadContent);
    
        // reads data from the file
        bool opRead (std::string path, std::string fileNameToWrite);

        // removes a file
        bool opDel (std::string path);

    };

} // namespace filesystem12

#endif // FAT12_H_