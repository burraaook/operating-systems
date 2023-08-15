#include "include/fat12.h"
#include "include/util.h"

using namespace filesystem12;

int main (int argc, char** argv) 
{
    // sample usage: ./fileSystemOper mysystem.dat <command> <command args>

    // check if arguments are valid
    if (argc > 5 || argc < 3)
    {
        std::cerr << "Usage: ./fileSystemOper <file name> <command> <command args>" << std::endl;
        return 1;
    } 

    // create a fat12 object
    fatTable *fat12 = new fatTable();

    // start the fat12 object
    if (!fat12->start(argv[1]))
    {
        delete fat12;
        return 1;
    }

    // check the command
    // dumpe2fs
    if (strcmp(argv[2], "dumpe2fs") == 0)
    {
        fat12->opDumpe2fs();
    }

    // dir
    else if (strcmp(argv[2], "dir") == 0)
    {
        if (argc != 4)
        {
            std::cerr << "Usage: ./fileSystemOper <file name> dir <path>" << std::endl;
            delete fat12;
            return 1;
        }

        fat12->opDir(argv[3]);
    }

    // mkdir
    else if (strcmp(argv[2], "mkdir") == 0)
    {
        if (argc != 4)
        {
            std::cerr << "Usage: ./fileSystemOper <file name> mkdir <path>" << std::endl;
            delete fat12;
            return 1;
        }

        fat12->opMkdir(argv[3]);
    }

    // rmdir
    else if (strcmp(argv[2], "rmdir") == 0)
    {
        if (argc != 4)
        {
            std::cerr << "Usage: ./fileSystemOper <file system> rmdir <path>" << std::endl;
            delete fat12;
            return 1;
        }

        fat12->opRmdir(argv[3]);
    }
    
    // write
    else if (strcmp(argv[2], "write") == 0)
    {
        if (argc != 5)
        {
            std::cerr << "Usage: ./fileSystemOper <file system> write <path> <file name>" << std::endl;
            delete fat12;
            return 1;
        }

        fat12->opWrite(argv[3], argv[4]);
    }

    // read
    else if (strcmp(argv[2], "read") == 0)
    {
        if (argc != 5)
        {
            std::cerr << "Usage: ./fileSystemOper <file system> read <path> <file name>" << std::endl;
            delete fat12;
            return 1;
        }

        fat12->opRead(argv[3], argv[4]);
    }

    // del
    else if (strcmp(argv[2], "del") == 0)
    {
        if (argc != 4)
        {
            std::cerr << "Usage: ./fileSystemOper <file system> del <path>" << std::endl;
            delete fat12;
            return 1;
        }

        fat12->opDel(argv[3]);
    }

    else 
    {
        std::cerr << "Error: Invalid command" << std::endl;
        delete fat12;
        return 1;
    }
    
    delete fat12;

    return 0;
}