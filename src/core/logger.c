#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include "logger.h"

void func_log(logLevel lvl, const char* msg) {
        const char* lvl_clr[5] = {"0;41", "1;31", "1;33", "1;32", "1;34"};
        const char* lvl_str[5] = {"[FATAL] | ",
                                  "[ERROR] | ",
                                  "[WARN] | ",
                                  "[INFO] | ",
                                  "[DEBUG] | "};
#if !defined MSUPPRESS
        printf("\033[%sm%s%s\033[0m\n", lvl_clr[lvl], lvl_str[lvl], msg);
#endif
}