# General
- Implementing a FAT-12 file system simulator in C++. It supports dir, mkdir, rmdir, dumpe2fs, write, read, del operations.  

Compilation
```
make
```

Run - Make file system
```
./makeFileSystem <block_size_in_KB> <file_name>
```	

Run - File system operations
```
./fileSystemOper <file_name> <operation> <parameters>
```

# Some implementation details
- Each block holds next block index.  
- Super block contains information about the file system such as magic number, root position, block size, etc.  
- Free blocks are stored in a linked list.  
- Block contains type information if it is a directory or a file.  
- Other details, and test results are in the doc.pdf file.  

# Warnings
- "\\" character is the separator since this is a DOS file system  
- "\\\\" character for root directory operations if you are testing on bash  

```
$ .\fileSystemOper fileSystem.data dir "\\"
```
- this displays the root directory  