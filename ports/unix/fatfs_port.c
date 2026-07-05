#include <time.h>
#include "lib/oofatfs/ff.h"

DWORD get_fattime(void) {
    time_t now = time(NULL);
    struct tm *tm = localtime(&now);
    return ((1900 + (uint32_t)tm->tm_year - 1980) << 25)
           | (((uint32_t)tm->tm_mon + 1) << 21)
           | ((uint32_t)tm->tm_mday << 16)
           | ((uint32_t)tm->tm_hour << 11)
           | ((uint32_t)tm->tm_min << 5)
           | ((uint32_t)tm->tm_sec / 2);
}
