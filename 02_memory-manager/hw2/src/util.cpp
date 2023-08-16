#include "../include/util.h"

bool stringToSizeT(std::string str, size_t* val)
{
    *val = 0;

    for (size_t i = 0; i < str.length(); i++)
    {
        if (str[i] < '0' || str[i] > '9')
        {
            return false;
        }

        *val *= 10;
        *val += str[i] - '0';
    }

    return true;
}
