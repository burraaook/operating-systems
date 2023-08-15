
#ifndef UTIL_H
#define UTIL_H

#include <string>
#include <iostream>
#include <cstring>

#define KB_FAT 1024
#define MB_FAT 1024*KB_FAT
#define MAGIC_NUMBER 15

bool stringToSizeT (std::string str, size_t* val);

#endif // UTIL_H