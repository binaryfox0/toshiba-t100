#include "MemoryView/Helpers.hpp"

#include <ctype.h>
#include <stdlib.h>

namespace MemoryView
{
bool CheckNumberRadix(const char *str, uint32_t *out_num)
{
    int out_base = 0;
    const char* backup = str;
    bool add = (str[0] == '-' || str[0] == '+');
    if(add)
        str++;
    if(str[0] == '0') {
        if(tolower(str[1]) == 'x') {
            out_base = 16;
            str += 2;
        } else
            out_base = 10;
    }
    auto func = out_base == 10 ? isdigit : isxdigit;
    char c;
    while(!isspace(c = *str) && c) {
        if(!func(c))
            return false;
        str++;
    }
    if(add)
        *out_num += strtol(backup, 0, out_base);
    else
        *out_num = strtol(backup, 0, out_base);
    return true;
}
}