#include "../include/fat12.h"

using namespace filesystem12;

// helper functions
bool writeFat12 (fat12_b& fat12, std::ofstream& file);

// directory functions
bool printDirectory (const directory& d);
bool printDirectoryEntry (const directoryEntry& d);

// binary writers
bool writeSuperblock (superblock& sb, std::ofstream& file);
bool writeBlock (block& b, std::ofstream& file);

// binary readers
bool readSuperblock (superblock& sb, std::ifstream& file);
bool readBlock (block& b, std::ifstream& file);

bool setDateTime (std::string& time, std::string& date);

fatTable::fatTable ()
{
    isInitialized = false;
}

// destructor
fatTable::~fatTable ()
{

}

bool fatTable::init (std::string blockSize)
{
    // check if already initialized
    if (isInitialized)
        return false;
    
    // check if block size is valid
    if (blockSize != "0.5" && blockSize != "1" && blockSize != "2" && blockSize != "4")
        return false;

    size_t mb = MB_FAT;
    size_t size_of_partition = 0;
    size_t size_of_block = 0;

    if (blockSize == "0.5")
    {
        size_of_partition = mb * 2;
        size_of_block = KB_FAT / 2;
    }
    else if (blockSize == "1")
    {
        size_of_partition = mb * 4;
        size_of_block = KB_FAT;
    }

    else if (blockSize == "2")
    {
        size_of_partition = mb * 8;
        size_of_block = KB_FAT * 2;
    }
    else if (blockSize == "4")
    {
        size_of_partition = mb * 16;
        size_of_block = KB_FAT * 4;
    }

    // initialize the superblock
    fat12.sb.blockSize = size_of_block;
    fat12.sb.numberOfBlocks = size_of_partition / size_of_block;
    fat12.sb.blockCount = 1; // root directory
    fat12.sb.magicNumber = MAGIC_NUMBER;
    fat12.sb.rootPosition = 0;
    fat12.sb.freeBlockCount = 0;
    fat12.sb.fileCount = 0;
    fat12.sb.directoryCount = 1; // root directory

    // create empty blocks
    for (size_t i = 1; i < fat12.sb.numberOfBlocks; i++)
    {
        block b;
        b.blockNumber = i;
        b.isDirectory = false;
        b.nextBlock = -1;
        // initialize data
        for (size_t j = 0; j < fat12.sb.blockSize; j++)
        {
            b.data.push_back('0');
        }
        fat12.freeBlocks.push_back(b);
        fat12.sb.freeBlockCount++;
    }

    // first block is reserved for root directory
    block b;
    b.blockNumber = 0;
    b.nextBlock = -1;
    b.isDirectory = true;
    // initialize data
    for (size_t j = 0; j < fat12.sb.blockSize; j++)
    {
        b.data.push_back('0');
    }
    std::vector<char> rootDirectoryData;
    
    // create root directory
    directory rootDirectory;
    rootDirectory.directoryPath = "\\";
    rootDirectory.numberOfEntries = 0;
    rootDirectory.blockNumber = 0;
    rootDirectory.entries = std::vector<directoryEntry>();

    serializeDirectory(rootDirectory, rootDirectoryData);

    // set root directory
    b.data = rootDirectoryData;
    fat12.blocks.push_back(b);

    isInitialized = true;
    return true;
}

bool fatTable::start (std::string fileName)
{
    // check if already initialized
    if (isInitialized)
        return false;

    // read the fat12 object
    if (!read(fileName))
    {
        std::cerr << "Error: File system image not found" << std::endl;
        return false;
    }

    fileSystemName = fileName;

    isInitialized = true;
    return true;
}

// write filesystem to file as binary
bool fatTable::write (std::string fileName)
{
    if (!isInitialized)
        return false;

    std::ofstream file;
    file.open(fileName, std::ios::out | std::ios::binary);
    
    fileSystemName = fileName;

    if (!file.is_open())
        return false;
    
    return writeFat12(fat12, file);
}

bool fatTable::read (std::string fileName)
{
    std::ifstream file;
    file.open(fileName, std::ios::in | std::ios::binary);

    if (!file.is_open())
        return false;
    
    return readFat12(fat12, file);
}

bool fatTable::opDir (std::string path)
{
    if (!isInitialized)
        return false;

    // check if path is valid
    if (path[0] != '\\')
    {
        std::cerr << "Error: Invalid path" << std::endl;
        return false;
    }

    // get root position from superblock
    size_t rootPosition = fat12.sb.rootPosition;

    // get root directory
    block rootDirectory = fat12.blocks[rootPosition];

    // deserialize root directory
    directory root;

    if (!deserializeDirectory(root, rootDirectory))
    {
        std::cerr << "Error: Could not deserialize root directory" << std::endl;
        return false;
    }

    // search for directory in root directory
    directory d;

    // get directory name to be searched
    std::string directoryName = path.substr(path.find_last_of("\\") + 1);

    if (!searchDirectory(root, path, d, directoryName))
    {
        return false;
    }

    // print directory and its contents
    printDirectory(d);

    return true;
}

bool fatTable::opMkdir (std::string path)
{
    if (!isInitialized)
        return false;

    // check if path is valid
    if (path[0] != '\\')
    {
        std::cerr << "Error1: Invalid path" << std::endl;
        return false;
    }

    // cannot create root directory
    if (path == "\\")
    {
        std::cerr << "Error: Cannot create root directory" << std::endl;
        return false;
    }

    // are there any free blocks
    if (fat12.sb.freeBlockCount == 0)
    {
        std::cerr << "Error: file system is full" << std::endl;
        return false;
    }

    std::string directoryPath = path;

    // get root position from superblock
    size_t rootPosition = fat12.sb.rootPosition;

    // get root directory
    block rootDirectory = fat12.blocks[rootPosition];

    // deserialize root directory
    directory root;

    if (!deserializeDirectory(root, rootDirectory))
    {
        std::cerr << "Error: Could not deserialize root directory" << std::endl;
        return false;
    }

    // get directory name to be created
    std::string directoryName = path.substr(path.find_last_of("\\") + 1);

    // discard directory name from path
    path = path.substr(0, path.find_last_of("\\"));

    // get last directory name
    std::string lastDirectoryName = path.substr(path.find_last_of("\\") + 1);


    directory lastDirectory;

    // search for last directory
    if (!searchDirectory(root, path, lastDirectory, lastDirectoryName))
    {
        std::cerr << "Error: Directory not found" << std::endl;
        return false;
    }

    // check if directory already exists
    for (size_t i = 0; i < lastDirectory.numberOfEntries; ++i)
    {
        if (lastDirectory.entries[i].fileName == directoryName)
        {
            std::cerr << "Error: Directory already exists" << std::endl;
            return false;
        }
    }

    // create directory entry
    directoryEntry dirEntry;
    dirEntry.fileName = directoryName;
    dirEntry.isDirectory = true;
    dirEntry.length = 0;
    dirEntry.extension = "0";
    dirEntry.time = "";
    dirEntry.date = "";

    if (fat12.freeBlocks.size() == 0)
    {
        std::cerr << "Error: File system is full" << std::endl;
        return false;
    }

    // get last block from free list
    block dirBlock = fat12.freeBlocks.back();

    // remove last block from free list
    fat12.freeBlocks.pop_back();

    // decrease free block count
    fat12.sb.freeBlockCount--;

    // set next block to -1
    dirBlock.nextBlock = -1;

    // set directory block number
    dirEntry.blockNumber = dirBlock.blockNumber;

    // set time and date
    if (!setDateTime(dirEntry.time, dirEntry.date))
    {
        std::cerr << "Error: Could not set time and date" << std::endl;
        return false;
    }

    // add directory entry to last directory
    lastDirectory.entries.push_back(dirEntry);

    // increase number of entries
    lastDirectory.numberOfEntries++;

    // update last directory
    if (!updateDirectory(lastDirectory))
    {
        std::cerr << "Error: Could not update last directory" << std::endl;
        return false;
    }

    // create directory
    directory newDirectory;
    newDirectory.directoryPath = directoryPath;
    newDirectory.numberOfEntries = 0;
    newDirectory.blockNumber = dirEntry.blockNumber;
    newDirectory.entries = std::vector<directoryEntry>();

    // serialize directory
    std::vector<char> newDirectoryData;
    
    if (!serializeDirectory(newDirectory, newDirectoryData))
    {
        std::cerr << "Error: Could not serialize new directory" << std::endl;
        return false;
    }

    // set directory block data
    dirBlock.data = newDirectoryData;

    // add directory block to blocks
    fat12.blocks.push_back(dirBlock);

    // increase block count
    fat12.sb.blockCount++;

    // increase directory count
    fat12.sb.directoryCount++;

    // update superblock
    if (!write(fileSystemName))
    {
        std::cerr << "Error: Could not write to binary file" << std::endl;
        return false;
    }

    return true;
}

bool fatTable::opRmdir (std::string path)
{
    if (!isInitialized)
        return false;

    // check if path is valid
    if (path[0] != '\\')
    {
        std::cerr << "Error: Invalid path" << std::endl;
        return false;
    }

    // cannot remove root directory
    if (path == "\\")
    {
        std::cerr << "Error: Cannot remove root directory" << std::endl;
        return false;
    }

    // get root position from superblock
    size_t rootPosition = fat12.sb.rootPosition;

    // get root directory
    block rootDirectory = fat12.blocks[rootPosition];

    // deserialize root directory
    directory root;

    if (!deserializeDirectory(root, rootDirectory))
    {
        std::cerr << "Error: Could not deserialize root directory" << std::endl;
        return false;
    }

    // get directory name to be removed
    std::string directoryName = path.substr(path.find_last_of("\\") + 1);

    // discard directory name from path
    path = path.substr(0, path.find_last_of("\\"));

    // get last directory name
    std::string lastDirectoryName = path.substr(path.find_last_of("\\") + 1);

    directory lastDirectory;

    // search for last directory
    if (!searchDirectory(root, path, lastDirectory, lastDirectoryName))
    {
        std::cerr << "Error: Directory not found" << std::endl;
        return false;
    }

    // check if directory exists
    bool exists = false;
    size_t index = 0;

    for (size_t i = 0; i < lastDirectory.numberOfEntries; ++i)
    {
        if (lastDirectory.entries[i].fileName == directoryName &&
            lastDirectory.entries[i].isDirectory == true)
        {
            exists = true;
            index = i;
            break;
        }
    }

    if (!exists)
    {
        std::cerr << "Error: Directory not found" << std::endl;
        return false;
    }

    // get directory entry
    directoryEntry dirEntry = lastDirectory.entries[index];

    // get directory block
    block dirBlock;

    if (!findBlock(dirEntry.blockNumber, dirBlock))
    {
        std::cerr << "Error: Could not find directory block" << std::endl;
        return false;
    }

    // deserialize directory
    directory dir;

    if (!deserializeDirectory(dir, dirBlock))
    {
        std::cerr << "Error: Could not deserialize directory" << std::endl;
        return false;
    }

    // check if directory is empty
    if (dir.numberOfEntries != 0)
    {
        std::cerr << "Error: Directory is not empty" << std::endl;
        return false;
    }
    else
    {
        // remove directory entry from last directory
        lastDirectory.entries.erase(lastDirectory.entries.begin() + index);

        // decrease number of entries
        lastDirectory.numberOfEntries--;

        // update last directory
        if (!updateDirectory(lastDirectory))
        {
            std::cerr << "Error: Could not update last directory" << std::endl;
            return false;
        }

        block removed;
        size_t removedIndex = 0;
        // remove directory block from blocks
        for (block b : fat12.blocks)
        {
            if (b.blockNumber == dirEntry.blockNumber)
            {
                // delete block
                removed = b;
                fat12.blocks.erase(fat12.blocks.begin() + removedIndex);
            }
            removedIndex++;
        }

        // add removed block to free list
        fat12.freeBlocks.push_back(removed);

        // increase free block count
        fat12.sb.freeBlockCount++;

        // decrease block count
        fat12.sb.blockCount--;

        // decrease directory count
        fat12.sb.directoryCount--;

        // update superblock
        if (!write(fileSystemName))
        {
            std::cerr << "Error: Could not write to binary file" << std::endl;
            return false;
        }
    }

    return true;
}

bool fatTable::opDumpe2fs ()
{
    if (!isInitialized)
        return false;

    // print file system information
    std::cout << "\nFAT-12 FILE SYSTEM \n" << std::endl;
    std::cout << "Block Size: " << fat12.sb.blockSize << " KB" << std::endl;
    std::cout << "Block Count: " << fat12.sb.blockCount << std::endl;
    std::cout << "Number of Blocks: " << fat12.sb.numberOfBlocks << std::endl;
    std::cout << "Number of Free Blocks: " << fat12.sb.freeBlockCount << std::endl;
    std::cout << "Number Of Files: " << fat12.sb.fileCount << std::endl;
    std::cout << "Number Of Directories: " << fat12.sb.directoryCount << std::endl;

    return true;
}

bool fatTable::opWrite (std::string path, std::string fileNameToReadContent)
{
    if (!isInitialized)
        return false;

    // check if path is valid
    if (path[0] != '\\')
    {
        std::cerr << "Error: Invalid path" << std::endl;
        return false;
    }

    // get root position from superblock
    size_t rootPosition = fat12.sb.rootPosition;

    // get root directory
    block rootDirectory = fat12.blocks[rootPosition];

    // deserialize root directory
    directory root;

    if (!deserializeDirectory(root, rootDirectory))
    {
        std::cerr << "Error: Could not deserialize root directory" << std::endl;
        return false;
    }
    
    // get file name to be created
    std::string fileName = path.substr(path.find_last_of("\\") + 1);

    // discard file name from path
    path = path.substr(0, path.find_last_of("\\"));

    // get last directory name
    std::string lastDirectoryName = path.substr(path.find_last_of("\\") + 1);

    directory lastDirectory;

    // search for last directory
    if (!searchDirectory(root, path, lastDirectory, lastDirectoryName))
    {
        std::cerr << "Error: Directory not found" << std::endl;
        return false;
    }

    bool exists = false;
    directoryEntry fileEntry;
    size_t index = 0;

    // check if file already exists
    for (size_t i = 0; i < lastDirectory.numberOfEntries; ++i)
    {
        if (lastDirectory.entries[i].isDirectory == false && 
            lastDirectory.entries[i].fileName == fileName)
        {
            exists = true;
            index = i;        
            break;
        }
    }

    // if file already exists, overwrite it
    if (exists)
    {
        // get file entry
        fileEntry = lastDirectory.entries[index];

        // set time and date
        if (!setDateTime(fileEntry.time, fileEntry.date))
        {
            std::cerr << "Error: Could not set time and date" << std::endl;
            return false;
        }

        // get file block
        block fileBlock;

        if (!findBlock(fileEntry.blockNumber, fileBlock))
        {
            std::cerr << "Error: Could not find file block" << std::endl;
            return false;
        }
        // deserialize file
        file fileToOverwrite;

        fileToOverwrite.fileName = fileName;
        fileToOverwrite.fileDate = fileEntry.date;
        fileToOverwrite.fileTime = fileEntry.time;

        // serialize file
        std::vector<char> newFileData;

        if (!serializeFile(fileToOverwrite, fileNameToReadContent, newFileData))
        {
            std::cerr << "Error: Could not serialize new file" << std::endl;
            return false;
        }

        // update file entry length
        fileEntry.length = fileToOverwrite.fileLength;
        fileToOverwrite.fileLength = fileToOverwrite.fileLength;

        // set file entry to last directory
        lastDirectory.entries[index] = fileEntry;

        // update last directory
        if (!updateDirectory(lastDirectory))
        {
            std::cerr << "Error: Could not update last directory" << std::endl;
            return false;
        }
        
        // check if file fits in the same block
        if (newFileData.size() <= fat12.sb.blockSize)
        {
            // set file block data
            fileBlock.data = newFileData;

            // set file block to blocks
            if (!setBlock(fileEntry.blockNumber, fileBlock))
            {
                std::cerr << "Error: Could not set file block" << std::endl;
                return false;
            }
        }
        else
        {
            // add file data to current block as much as possible
            for (size_t i = 0; i < fat12.sb.blockSize; i++)
            {
                fileBlock.data[i] = newFileData[i];
            }

            size_t remainingBytes = newFileData.size() - fat12.sb.blockSize;
            // discard new file data that is already written
            newFileData.erase(newFileData.begin(), newFileData.begin() + fat12.sb.blockSize);

            bool done = false;
            while (true)
            {
                // check if there is a next block
                if (fileBlock.nextBlock == -1 && !done)
                {
                    
                    // check if there are any free blocks
                    if (fat12.freeBlocks.size() == 0)
                    {
                        std::cerr << "Error: File system is full" << std::endl;
                        return false;
                    }

                    // get next block from free list
                    block nextBlock = fat12.freeBlocks.back();

                    // remove last block from free list
                    fat12.freeBlocks.pop_back();

                    // decrease free block count
                    fat12.sb.freeBlockCount--;

                    // set next block to -1
                    nextBlock.nextBlock = -1;

                    // set next block of current block
                    fileBlock.nextBlock = nextBlock.blockNumber;

                    // set file block to blocks
                    if (!setBlock(fileBlock.blockNumber, fileBlock))
                    {
                        std::cerr << "Error: Could not set file block" << std::endl;
                        return false;
                    }

                    // set file block number
                    fileBlock = nextBlock;

                    // add file data to current block as much as possible
                    for (size_t i = 0; i < fat12.sb.blockSize; i++)
                    {
                        fileBlock.data[i] = newFileData[i];
                    }

                    // // discard new file data that is already written
                    newFileData.erase(newFileData.begin(), newFileData.begin() + fat12.sb.blockSize);

                    // add next block to blocks
                    fat12.blocks.push_back(fileBlock);

                    // increase block count
                    fat12.sb.blockCount++;
                }
                else if (!done)
                {
                    // set file block to blocks
                    if (!setBlock(fileBlock.blockNumber, fileBlock))
                    {
                        std::cerr << "Error: Could not set file block" << std::endl;
                        return false;
                    }

                    // get next block
                    if (!findBlock(fileBlock.nextBlock, fileBlock))
                    {
                        std::cerr << "Error: Could not find file block" << std::endl;
                        return false;
                    }

                    // add file data to current block as much as possible
                    for (size_t i = 0; i < fat12.sb.blockSize; i++)
                    {
                        fileBlock.data[i] = newFileData[i];
                    }

                    // discard new file data that is already written
                    newFileData.erase(newFileData.begin(), newFileData.begin() + fat12.sb.blockSize);
                }

                if (done)
                    break;

                // consider underflow
                if (remainingBytes <= fat12.sb.blockSize)
                {
                    done = true;
                    continue;
                }

                remainingBytes -= fat12.sb.blockSize;
            }
        }

        // update superblock
        if (!write(fileSystemName))
        {
            std::cerr << "Error: Could not write to binary file" << std::endl;
            return false;
        }

    }
    else
    {
        // create file entry
        directoryEntry fileEntry;
        fileEntry.fileName = fileName;
        fileEntry.isDirectory = false;
        fileEntry.length = 0;
        fileEntry.extension = "0";
        fileEntry.time = "";
        fileEntry.date = "";

        if (fat12.freeBlocks.size() == 0)
        {
            std::cerr << "Error: File system is full" << std::endl;
            return false;
        }

        // get last block from free list
        block fileBlock = fat12.freeBlocks.back();

        // remove last block from free list
        fat12.freeBlocks.pop_back();

        // decrease free block count
        fat12.sb.freeBlockCount--;
        
        // set next block to -1
        fileBlock.nextBlock = -1;

        // set file block number
        fileEntry.blockNumber = fileBlock.blockNumber;

        // set time and date
        if (!setDateTime(fileEntry.time, fileEntry.date))
        {
            std::cerr << "Error: Could not set time and date" << std::endl;
            return false;
        }

        // create file
        file newFile;
        newFile.fileName = fileName;
        newFile.fileLength = 0;
        newFile.blockNumber = fileEntry.blockNumber;

        // set time and date
        newFile.fileTime = fileEntry.time;
        newFile.fileDate = fileEntry.date;

        // serialize file
        std::vector<char> newFileData;

        if (!serializeFile(newFile, fileNameToReadContent, newFileData))
        {
            std::cerr << "Error: Could not serialize new file" << std::endl;
            return false;
        }

        // update file entry length
        fileEntry.length = newFile.fileLength;
        newFile.fileLength = newFile.fileLength;

        // add file entry to last directory
        lastDirectory.entries.push_back(fileEntry);

        // increase number of entries
        lastDirectory.numberOfEntries++;

        // update last directory
        if (!updateDirectory(lastDirectory))
        {
            std::cerr << "Error: Could not update last directory" << std::endl;
            return false;
        }

        // if file fits in the same block
        if (newFileData.size() <= fat12.sb.blockSize)
        {
            // set file block data
            fileBlock.data = newFileData;

            // add file block to blocks
            fat12.blocks.push_back(fileBlock);
            fat12.sb.blockCount++;
        }
        else
        {

            fat12.sb.blockCount++;
            // add file data to current block as much as possible
            for (size_t i = 0; i < fat12.sb.blockSize; i++)
            {
                fileBlock.data[i] = newFileData[i];
            }

            // discard new file data that is already written
            newFileData.erase(newFileData.begin(), newFileData.begin() + fat12.sb.blockSize);

            fat12.blocks.push_back(fileBlock);
            size_t remainingBytes = newFileData.size() - fat12.sb.blockSize;
            bool done = false;
            while (true)
            {
                if (fat12.freeBlocks.size() == 0)
                {
                    std::cerr << "Error: File system is full" << std::endl;
                    return false;
                }

                // get next block from free list
                block nextBlock = fat12.freeBlocks.back();

                // remove last block from free list
                fat12.freeBlocks.pop_back();

                // decrease free block count
                fat12.sb.freeBlockCount--;

                // set next block to -1
                nextBlock.nextBlock = -1;

                // set next block of current block
                fileBlock.nextBlock = nextBlock.blockNumber;

                // set file block to blocks
                if (!setBlock(fileBlock.blockNumber, fileBlock))
                {
                    std::cerr << "Error: Could not set file block" << std::endl;
                    return false;
                }

                // set file block number
                fileBlock = nextBlock;

                // add file data to current block as much as possible
                for (size_t i = 0; i < fat12.sb.blockSize; i++)
                {
                    fileBlock.data[i] = newFileData[i];
                }

                // discard new file data that is already written
                if (!done)
                    newFileData.erase(newFileData.begin(), newFileData.begin() + fat12.sb.blockSize);

                // add next block to blocks
                fat12.blocks.push_back(fileBlock);

                // increase block count
                fat12.sb.blockCount++;

                if (done)
                    break;

                // consider underflow
                if (remainingBytes <= fat12.sb.blockSize)
                {  
                    done = true;
                    continue;
                }

                remainingBytes -= fat12.sb.blockSize;
            }
        }

        // increase file count
        fat12.sb.fileCount++;

        // update superblock
        if (!write(fileSystemName))
        {
            std::cerr << "Error: Could not write to binary file" << std::endl;
            return false;
        }
    }


    return true;
}

// write the content of a file in the path to fileNameToWrite
bool fatTable::opRead (std::string path, std::string fileNameToWrite)
{
    if (!isInitialized)
        return false;

    // check if path is valid
    if (path[0] != '\\')
    {
        std::cerr << "Error: Invalid path" << std::endl;
        return false;
    }

    // get root position from superblock
    size_t rootPosition = fat12.sb.rootPosition;

    // get root directory
    block rootDirectory = fat12.blocks[rootPosition];

    // deserialize root directory
    directory root;

    if (!deserializeDirectory(root, rootDirectory))
    {
        std::cerr << "Error: Could not deserialize root directory" << std::endl;
        return false;
    }

    // get file name to be created
    std::string fileName = path.substr(path.find_last_of("\\") + 1);

    // discard file name from path
    path = path.substr(0, path.find_last_of("\\"));

    // get last directory name
    std::string lastDirectoryName = path.substr(path.find_last_of("\\") + 1);

    directory lastDirectory;

    // search for last directory
    if (!searchDirectory(root, path, lastDirectory, lastDirectoryName))
    {
        std::cerr << "Error: Directory not found" << std::endl;
        return false;
    }

    directoryEntry fileEntry;
    bool exists = false;
    // check if file exists, if it does not, return false
    for (size_t i = 0; i < lastDirectory.numberOfEntries; ++i)
    {
        if (lastDirectory.entries[i].isDirectory == false && 
            lastDirectory.entries[i].fileName == fileName)
        {
            fileEntry = lastDirectory.entries[i];
            exists = true;
            break;
        }
    }

    if (!exists)
    {
        std::cerr << "Error: File does not exist" << std::endl;
        return false;
    }

    // get file block
    block fileBlock;

    if (!findBlock(fileEntry.blockNumber, fileBlock))
    {
        std::cerr << "Error: Could not find file block" << std::endl;
        return false;
    }

    // deserialize file, and write to fileNameToWrite
    if (!writeFileContent(fileBlock, fileNameToWrite, fileEntry.length))
    {
        std::cerr << "Error: Could not write file content" << std::endl;
        return false;
    }

    return true;
}

bool fatTable::opDel (std::string path)
{
    if (!isInitialized)
        return false;

    // check if path is valid
    if (path[0] != '\\')
    {
        std::cerr << "Error: Invalid path" << std::endl;
        return false;
    }

    // cannot remove root directory
    if (path == "\\")
    {
        std::cerr << "Error: Cannot remove root directory" << std::endl;
        return false;
    }

    // get root position from superblock
    size_t rootPosition = fat12.sb.rootPosition;

    // get root directory
    block rootDirectory = fat12.blocks[rootPosition];

    // deserialize root directory
    directory root;

    if (!deserializeDirectory(root, rootDirectory))
    {
        std::cerr << "Error: Could not deserialize root directory" << std::endl;
        return false;
    }

    // get file name to be removed
    std::string fileName = path.substr(path.find_last_of("\\") + 1);

    // discard file name from path
    path = path.substr(0, path.find_last_of("\\"));

    // get last directory name
    std::string lastDirectoryName = path.substr(path.find_last_of("\\") + 1);
    directory lastDirectory;

    // search for last directory
    if (!searchDirectory(root, path, lastDirectory, lastDirectoryName))
    {
        std::cerr << "Error: Directory not found" << std::endl;
        return false;
    }

    // check if file exists
    bool exists = false;
    size_t index = 0;

    for (size_t i = 0; i < lastDirectory.numberOfEntries; ++i)
    {
        if (lastDirectory.entries[i].fileName == fileName
            && lastDirectory.entries[i].isDirectory == false)
        {
            exists = true;
            index = i;
            break;
        }
    }

    if (!exists)
    {
        std::cerr << "Error: File not found" << std::endl;
        return false;
    }

    // get file entry
    directoryEntry fileEntry;

    for (size_t i = 0; i < lastDirectory.numberOfEntries; ++i)
    {
        if (lastDirectory.entries[i].fileName == fileName)
        {
            fileEntry = lastDirectory.entries[i];
            break;
        }
    }

    // get file block
    block fileBlock;
    if (!findBlock(fileEntry.blockNumber, fileBlock))
    {
        std::cerr << "Error: Could not find file block" << std::endl;
        return false;
    }

    // remove file entry from last directory
    lastDirectory.entries.erase(lastDirectory.entries.begin() + index);

    // decrease number of entries
    lastDirectory.numberOfEntries--;

    // update last directory
    if (!updateDirectory(lastDirectory))
    {
        std::cerr << "Error: Could not update last directory" << std::endl;
        return false;
    }

    // remove file block, and its next blocks from blocks, block count and free block count updated in removeFile
    if (!removeFile(fileBlock))
    {
        std::cerr << "Error: Could not remove file" << std::endl;
        return false;
    }

    // decrease file count
    fat12.sb.fileCount--;

    // update superblock
    if (!write(fileSystemName))
    {
        std::cerr << "Error: Could not write to binary file" << std::endl;
        return false;
    }

    return true;
}

// helper functions
bool writeFat12 (fat12_b& fat12, std::ofstream& file)
{
    // write superblock
    if (!writeSuperblock(fat12.sb, file))
        return false;
    
    // write blocks
    for (size_t i = 0; i < fat12.blocks.size(); i++)
    { 
        if (!writeBlock(fat12.blocks[i], file))
            return false;
    }


    // write free blocks
    for (size_t i = 0; i < fat12.freeBlocks.size(); i++)
    {
        if (!writeBlock(fat12.freeBlocks[i], file))
            return false;
    }

    return true;
}

bool fatTable::readFat12 (fat12_b& fat12, std::ifstream& file)
{
    superblock sb;
    // read superblock
    if (!readSuperblock(sb, file))
        return false;

    // set superblock
    fat12.sb = sb;

    // allocate blocks
    for (size_t i = 0; i < fat12.sb.blockCount; i++)
    {
        block b;
        // dummy initialization
        b.blockNumber = i;
        b.isDirectory = false;
        b.nextBlock = -1;
        // initialize data
        for (size_t j = 0; j < fat12.sb.blockSize; j++)
        {
            b.data.push_back('0');
        }

        fat12.blocks.push_back(b);
    }

    // allocate free blocks
    for (size_t i = 0; i < fat12.sb.freeBlockCount; i++)
    {
        block b;
        // dummy initialization
        b.blockNumber = i;
        b.isDirectory = false;
        b.nextBlock = -1;
        // initialize data
        for (size_t j = 0; j < fat12.sb.blockSize; j++)
        {
            b.data.push_back('0');
        }

        fat12.freeBlocks.push_back(b);
    }

    // read blocks
    for (size_t i = 0; i < fat12.blocks.size(); i++)
    {
        if (!readBlock(fat12.blocks[i], file))
            return false;
    }

    // read free blocks
    for (size_t i = 0; i < fat12.freeBlocks.size(); i++)
    {
        if (!readBlock(fat12.freeBlocks[i], file))
            return false;
    }
    
    return true;
}

bool writeBlock (block& b, std::ofstream& file)
{
    // write block struct to binary file

    if (!file.is_open())
        return false;

    // write block number
    file.write((char*)&b.blockNumber, sizeof(b.blockNumber));

    // write next block number
    file.write((char*)&b.nextBlock, sizeof(b.nextBlock));

    // write is directory it is a boolean type
    file.write((char*)&b.isDirectory, sizeof(b.isDirectory));

    // write data
    for (size_t i = 0; i < b.data.size(); i++)
    {
        file.write((char*)&b.data[i], sizeof(b.data[i]));
    }

    return true;
}

bool readBlock (block& b, std::ifstream& file)
{
    // read block struct from binary file

    if (!file.is_open())
        return false;

    // read block number
    file.read((char*)&b.blockNumber, sizeof(b.blockNumber));

    // read next block number
    file.read((char*)&b.nextBlock, sizeof(b.nextBlock));

    // read is directory it is a boolean type
    file.read((char*)&b.isDirectory, sizeof(b.isDirectory));

    // read data
    for (size_t i = 0; i < b.data.size(); i++)
    {
        file.read((char*)&b.data[i], sizeof(b.data[i]));
    }

    return true;
}

bool writeSuperblock (superblock& sb, std::ofstream& file)
{
    // write superblock struct to binary file

    if (!file.is_open())
        return false;
    
    file.write((char*)&sb, sizeof(superblock));
    return true;
}

bool readSuperblock (superblock& sb, std::ifstream& file)
{
    // read superblock struct from binary file

    if (!file.is_open())
        return false;

    file.read((char*)&sb, sizeof(superblock));
    return true;
}

bool fatTable::serializeDirectory (const directory& d, std::vector<char>& data)
{
    // serialize directory struct to vector of chars

    std::string content = "";

    // directory path
    content += d.directoryPath;
    content += "\n";

    // number of entries
    content += std::to_string(d.numberOfEntries);
    content += "\n";

    // block number
    content += std::to_string(d.blockNumber);
    content += "\n";

    // entries
    for (size_t i = 0; i < d.entries.size(); ++i)
    {
        // file name
        content += d.entries[i].fileName;
        content += "\n";

        // is directory
        content += std::to_string(d.entries[i].isDirectory);
        content += "\n";

        // length
        content += std::to_string(d.entries[i].length);
        content += "\n";

        // extension
        content += d.entries[i].extension;
        content += "\n";

        // time
        content += d.entries[i].time;
        content += "\n";

        // date
        content += d.entries[i].date;
        content += "\n";

        // block number
        content += std::to_string(d.entries[i].blockNumber);
        content += "\n";
    }

    data.clear();

    // convert content to vector of chars
    for (size_t i = 0; i < content.size(); i++)
    {
        data.push_back(content[i]);
    }

    // fill the rest of the block with zeros
    for (size_t i = content.size(); i < fat12.sb.blockSize; i++)
    {
        data.push_back('0');
    }

    return true;
}

bool fatTable::deserializeDirectory (directory& d, const block& directoryBlock)
{
    // deserialize directory struct from block

    // get data from block
    std::vector<char> data = directoryBlock.data;

    // convert data to string
    std::string content(data.begin(), data.end());

    block tempBlock = directoryBlock;

    // complete the content
    while (tempBlock.nextBlock != -1)
    {
        // get next block
        block nextBlock;

        if (!findBlock(tempBlock.nextBlock, nextBlock))
        {
            std::cerr << "Error: Could not find next block" << std::endl;
            return false;
        }

        // get data from next block
        data = nextBlock.data;
        // convert data to string
        std::string content2(data.begin(), data.end());

        // append content2 to content
        content += content2;

        tempBlock = nextBlock;
    }

    // parse content
    std::istringstream iss(content);

    // directory path
    std::getline(iss, d.directoryPath);

    // number of entries
    std::string numberOfEntries;

    std::getline(iss, numberOfEntries);

    // convert string to size_t
    size_t numberOfEntriesSizeT = 0;
    if (!stringToSizeT(numberOfEntries, &numberOfEntriesSizeT))
        return false;
    
    d.numberOfEntries = numberOfEntriesSizeT;

    // block number
    std::string blockNumber;

    std::getline(iss, blockNumber);

    // convert string to size_t
    size_t blockNumberSizeT = 0;
    if (!stringToSizeT(blockNumber, &blockNumberSizeT))
        return false;

    d.blockNumber = blockNumberSizeT;

    // entries
    for (size_t i = 0; i < d.numberOfEntries; ++i)
    {
        // push back empty entry
        d.entries.push_back(directoryEntry());

        // file name
        std::string fileName;
        std::getline(iss, fileName);

        // is directory
        std::string isDirectory;
        std::getline(iss, isDirectory);
        d.entries[i].isDirectory = (isDirectory == "1");

        // length
        std::string length;
        std::getline(iss, length);

        // convert string to size_t
        size_t lengthSizeT = 0;

        if (!stringToSizeT(length, &lengthSizeT))
            return false;

        d.entries[i].length = lengthSizeT;

        // extension
        std::getline(iss, d.entries[i].extension);

        // time
        std::string time;
        std::getline(iss, time);
        d.entries[i].time = time;

        // date
        std::string date;
        std::getline(iss, date);
        d.entries[i].date = date;

        // block number
        std::string blockNumber;
        std::getline(iss, blockNumber);

        // convert string to size_t
        size_t blockNumberSizeT = 0;

        if (!stringToSizeT(blockNumber, &blockNumberSizeT))
            return false;

        d.entries[i].blockNumber = blockNumberSizeT;

        // file name
        d.entries[i].fileName = fileName;
    }


    return true;
}

// search directory starting from d, last ..\dirname is the directory to be searched
bool fatTable::searchDirectory (directory d, std::string path, directory& result, const std::string& directoryName)
{    
    // check if path does not contain any \ symbol
    if (path == "\\" || path.empty())
    {
        // set result to d
        result = d;
        return true;
    }

    // discard root directory from path
    if (path[0] == '\\')
        path = path.substr(1);
    
    // get next directory name
    std::string nextDirectoryName = path.substr(0, path.find_first_of("\\"));

    // discard next directory name from path
    if (path.find_first_of("\\") != std::string::npos)
        path = path.substr(path.find_first_of("\\") + 1);
    else
        path = "";

    // search for next directory name in current directory
    bool found = false;
    for (size_t i = 0; i < d.numberOfEntries; ++i)
    {
        if (d.entries[i].fileName == nextDirectoryName)
        {
            // found
            found = true;

            // check if it is a directory
            if (!d.entries[i].isDirectory)
            {
                std::cerr << "Error: " << nextDirectoryName << " is not a directory" << std::endl;
                return false;
            }

            // get next directory
            block nextDirectoryBlock;
            if (!findBlock(d.entries[i].blockNumber, nextDirectoryBlock))
            {
                std::cerr << "Error: Could not find next directory block" << std::endl;
                return false;
            }

            // deserialize next directory
            directory nextDirectory;
            if (!deserializeDirectory(nextDirectory, nextDirectoryBlock))
            {
                std::cerr << "Error: Could not deserialize next directory" << std::endl;
                return false;
            }

            // call with next directory
            return searchDirectory(nextDirectory, path, result, directoryName);
            break;
        }
    }

    if (!found)
    {
        std::cerr << "Error: Directory not found" << std::endl;
        return false;
    }

    // recursive call
    return searchDirectory(d, path, result, directoryName);
}
bool printDirectory (const directory& d)
{
    std::cout << "\nDirectory: " << d.directoryPath << std::endl;

    std::cout << "TYPE\t\tLAST WRITE\t\t\tLENGTH\tNAME" << std::endl;
    std::cout << "----\t\t----------\t\t\t------\t----" << std::endl;

    for (size_t i = 0; i < d.numberOfEntries; ++i)
    {
        std::cout << (d.entries[i].isDirectory ? "DIR" : "FILE") << "\t\t" << d.entries[i].date << " " << d.entries[i].time << "\t\t" << d.entries[i].length << "\t" << d.entries[i].fileName;
        if (!d.entries[i].isDirectory && d.entries[i].extension != "0")
            std::cout << "." << d.entries[i].extension;
        std::cout << std::endl;
    }
    return true;
}

bool setDateTime(std::string& time, std::string& date)
{
    // Get the current system time
    std::time_t currentTime = std::time(nullptr);
    std::tm* localTime = std::localtime(&currentTime);

    // Format the time as HH:MM
    char timeBuffer[6];
    std::strftime(timeBuffer, sizeof(timeBuffer), "%H:%M", localTime);
    time = timeBuffer;

    // Format the date as DD.MM.YYYY
    char dateBuffer[11];
    std::strftime(dateBuffer, sizeof(dateBuffer), "%d.%m.%Y", localTime);
    date = dateBuffer;

    return true;
}

bool printDirectoryEntry (const directoryEntry& d)
{
    std::cout << "\nTYPE\t\tLAST WRITE\t\tLENGTH\tNAME" << std::endl;
    std::cout << "----\t\t----------\t\t------\t----" << std::endl;

    std::cout << (d.isDirectory ? "DIR" : "FILE") << "\t\t" << d.date << " " << d.time << "\t\t" << d.length << "\t" << d.fileName << "." << d.extension << std::endl;

    return true;
}

bool fatTable::updateDirectory (directory& d)
{
    // update directory in blocks

    // get directory block
    block directoryBlock;
    if (!findBlock(d.blockNumber, directoryBlock))
    {
        std::cerr << "Error: Could not find directory block" << std::endl;
        return false;
    }

    // serialize directory
    std::vector<char> directoryData;
    if (!serializeDirectory(d, directoryData))
    {
        std::cerr << "Error: Could not serialize directory" << std::endl;
        return false;
    }

    // if directory data fits in the same block
    if (directoryData.size() <= fat12.sb.blockSize)
    {
        // set data
        directoryBlock.data = directoryData;

        // update directory block
        if (!setBlock(d.blockNumber, directoryBlock))
        {
            std::cerr << "Error: Could not update directory block" << std::endl;
            return false;
        }
    }
    else
    {
        // add directory data to current block as much as possible
        for (size_t i = 0; i < fat12.sb.blockSize; i++)
        {
            directoryBlock.data[i] = directoryData[i];
        }

        // discard directory data that is already written
        directoryData.erase(directoryData.begin(), directoryData.begin() + fat12.sb.blockSize);

        // add directory block to blocks
        fat12.blocks.push_back(directoryBlock);

        // increase block count
        fat12.sb.blockCount++;

        size_t remainingBytes = directoryData.size() - fat12.sb.blockSize;
        bool done = false;
        while (true)
        {
            if (fat12.freeBlocks.size() == 0)
            {
                std::cerr << "Error: File system is full" << std::endl;
                return false;
            }

            // get next block from free list
            block nextBlock = fat12.freeBlocks.back();

            // remove last block from free list
            fat12.freeBlocks.pop_back();

            // decrease free block count
            fat12.sb.freeBlockCount--;

            // set next block to -1
            nextBlock.nextBlock = -1;

            // set next block of current block
            directoryBlock.nextBlock = nextBlock.blockNumber;

            // set directory block to blocks
            if (!setBlock(directoryBlock.blockNumber, directoryBlock))
            {
                std::cerr << "Error: Could not set directory block" << std::endl;
                return false;
            }

            // set directory block number
            directoryBlock = nextBlock;

            // add directory data to current block as much as possible
            for (size_t i = 0; i < fat12.sb.blockSize; i++)
            {
                directoryBlock.data[i] = directoryData[i];
            }

            // discard directory data that is already written
            if (!done)
                directoryData.erase(directoryData.begin(), directoryData.begin() + fat12.sb.blockSize);

            // add next block to blocks
            fat12.blocks.push_back(directoryBlock);

            // increase block count
            fat12.sb.blockCount++;

            if (done)
                break;

            // consider underflow
            if (remainingBytes <= fat12.sb.blockSize)
            {  
                done = true;
                continue;
            }

            remainingBytes -= fat12.sb.blockSize;
        }
    }
    return true;
}

bool fatTable::findBlock (size_t blockNumber, block& b)
{
    // find block with block number
    for (size_t i = 0; i < fat12.blocks.size(); ++i)
    {
        if (fat12.blocks[i].blockNumber == blockNumber)
        {
            b = fat12.blocks[i];
            return true;
        }
    }

    return false;
}

bool fatTable::setBlock (size_t blockNumber, const block& b)
{
    // set block with block number
    for (size_t i = 0; i < fat12.blocks.size(); ++i)
    {
        if (fat12.blocks[i].blockNumber == blockNumber)
        {
            fat12.blocks[i] = b;
            fat12.blocks[i].data = b.data;
            return true;
        }
    }

    return false;
}

bool fatTable::deserializeFile (file& f, const block& fileBlock)
{
    // deserialize file struct from block

    // get data from block
    std::vector<char> data = fileBlock.data;

    // convert data to string
    std::string content(data.begin(), data.end());

    block tempBlock = fileBlock;

    // complete the content
    while (tempBlock.nextBlock != -1)
    {
        // get next block
        block nextBlock;

        if (!findBlock(tempBlock.nextBlock, nextBlock))
        {
            std::cerr << "Error: Could not find next block" << std::endl;
            return false;
        }

        // get data from next block
        std::vector<char> data = nextBlock.data;

        // convert data to string
        std::string content2(data.begin(), data.end());

        // append content2 to content
        content += content2;

        tempBlock = nextBlock;
    }

    // parse content
    std::istringstream iss(content);

    // file name
    std::getline(iss, f.fileName);

    // file extension
    std::getline(iss, f.fileExtension);

    // file time
    std::getline(iss, f.fileTime);

    // file date
    std::getline(iss, f.fileDate);

    // file length
    std::string fileLength;

    std::getline(iss, fileLength);

    // convert string to size_t
    size_t fileLengthSizeT = 0;
    if (!stringToSizeT(fileLength, &fileLengthSizeT))
        return false;
    
    f.fileLength = fileLengthSizeT;

    // block number
    std::string blockNumber;

    std::getline(iss, blockNumber);

    // convert string to size_t
    size_t blockNumberSizeT = 0;
    if (!stringToSizeT(blockNumber, &blockNumberSizeT))
        return false;

    f.blockNumber = blockNumberSizeT;
    return true;
}

bool fatTable::serializeFile (file &f, const std::string& fileNameToRead, std::vector<char>& data)
{
    // serialize file struct to vector of chars

    std::string content = "";

    // set to file length size from fileNameToRead
    // open file
    std::ifstream file(fileNameToRead, std::ios::binary);

    if (!file.is_open())
    {
        std::cerr << "Error: Could not open file: " << fileNameToRead << std::endl;
        return false;
    }

    // get file length
    file.seekg(0, std::ios::end);
    size_t fileLength = file.tellg();
    file.seekg(0, std::ios::beg);
    f.fileLength = fileLength;

    // read file content
    std::vector<char> fileContent(fileLength);
    file.read(fileContent.data(), fileLength);

    // convert file content to string
    std::string fileContentString(fileContent.begin(), fileContent.end());

    // append file content to content
    content += fileContentString;

    // convert content to vector of chars
    for (size_t i = 0; i < content.size(); i++)
    {
        data.push_back(content[i]);
    }

    // fill the rest of the block with zeros
    if (content.size() % fat12.sb.blockSize != 0)
    {
        for (size_t i = content.size() % fat12.sb.blockSize; i < fat12.sb.blockSize; i++)
        {
            data.push_back('0');
        }
    }

    return true;
}

bool fatTable::writeFileContent (const block& fileBlock, std::string fileNameToWrite, size_t fileLength)
{
    // write file content to fileNameToWrite

    // get data from block
    std::vector<char> data = fileBlock.data;

    // convert data to string
    std::string content(data.begin(), data.end());

    block tempBlock = fileBlock;

    // complete the content
    while (tempBlock.nextBlock != -1)
    {
        // get next block
        block nextBlock;

        if (!findBlock(tempBlock.nextBlock, nextBlock))
        {
            std::cerr << "Error: Could not find next block" << std::endl;
            return false;
        }

        // get data from next block
        data = nextBlock.data;
        // convert data to string
        std::string content2(data.begin(), data.end());

        // append content2 to content
        content += content2;

        tempBlock = nextBlock;
    }

    // get content as much as file length
    content = content.substr(0, fileLength);
    

    // write content to file
    std::ofstream file(fileNameToWrite, std::ios::binary);

    if (!file.is_open())
    {
        std::cerr << "Error: Could not open file: " << fileNameToWrite << std::endl;
        return false;
    }

    // write content to file
    file.write(content.c_str(), content.size());

    file.close();

    return true;
}

bool fatTable::removeFile (block& startBlock)
{
    // remove file starting from startBlock

    // get next block
    block nextBlock;
    if (startBlock.nextBlock == -1)
    {
        // remove start block and return
        if (!removeBlock(startBlock))
        {
            std::cerr << "Error: Could not remove start block" << std::endl;
            return false;
        }

        return true;
    }

    if (!findBlock(startBlock.nextBlock, nextBlock))
    {
        std::cerr << "Error: Could not find next block" << std::endl;
        return false;
    }

    // remove start block
    if (!removeBlock(startBlock))
    {
        std::cerr << "Error: Could not remove start block" << std::endl;
        return false;
    }

    // recursive call
    return removeFile(nextBlock);
}

bool fatTable::removeBlock (block& b)
{
    // remove block from blocks

    // get block number
    size_t blockNumber = b.blockNumber;

    // remove block from blocks
    for (size_t i = 0; i < fat12.blocks.size(); ++i)
    {
        if (fat12.blocks[i].blockNumber == blockNumber)
        {
            fat12.blocks.erase(fat12.blocks.begin() + i);
            break;
        }
    }

    b.nextBlock = -1;

    // add block to free blocks
    fat12.freeBlocks.push_back(b);

    // increase free block count
    fat12.sb.freeBlockCount++;

    // decrease block count
    fat12.sb.blockCount--;

    return true;
}