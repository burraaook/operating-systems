
#ifndef UTIL_H
#define UTIL_H

#include <string>
#include <iostream>
#include <cstring>
#include <chrono>
#include <thread>
#include <random>
#include <pthread.h>

enum class PageAlgorithm {
    SC,
    LRU,
    WSCLOCK
};

enum class PageTableType
{
    INVERTED,
    REGULAR
};

enum class ReplaceMode
{
    APPEND,
    OVERWRITE_AND_READ,
    READ_AND_DELETE
};

bool stringToSizeT (std::string str, size_t* val);

#endif // UTIL_H