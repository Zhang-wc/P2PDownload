#ifndef __M_UTILS_H__
#define __M_UTILS_H__ 

#include <iostream>
#include <string>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>

class Utils
{
    public:
        static bool GetRealpath(std::string &src) {
            char tmp[PATH_MAX] = {0};
            if (realpath(src.c_str(), tmp) == NULL) {
                return false;
            }
            src = tmp;
            return true;
        }
};

#endif
