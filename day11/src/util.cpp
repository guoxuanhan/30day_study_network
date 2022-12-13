#include <stdio.h>  // 使用到了perror函数
#include <stdlib.h> // 使用到了exit函数
#include "util.h"

void errif(bool condition, const char* errmsg) {
    if (condition)
    {
        perror(errmsg);
        exit(EXIT_FAILURE);
    }
}