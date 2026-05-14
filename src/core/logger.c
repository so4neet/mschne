#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "logger.h"
#include "../globals.h"

static void func_log(logLevel lvl, const char* msg, va_list args) {
        const char* lvl_clr[5] = {"0;41", "1;31", "1;33", "1;32", "1;34"};
        const char* lvl_str[5] = {"[FATAL] | ",
                                  "[ERROR] | ",
                                  "[WARN] | ",
                                  "[INFO] | ",
                                  "[DEBUG] | "};
        char buffer[4096];
        vsnprintf(buffer, sizeof(buffer), msg, args);
#if !defined MSUPPRESS
        if (!(lvl == LOG_LVL_FATAL)) {
                printf("\033[%sm%s%s\033[0m\n", lvl_clr[lvl], lvl_str[lvl], buffer);
        }
#endif
        if (lvl == LOG_LVL_FATAL) {
                printf("\033[%sm%s%s\033[0m\n", lvl_clr[lvl], lvl_str[lvl], buffer);                
        }
}

MAPI void mWarn(const char* msg, ...) {
        va_list args;
        va_start(args, msg);
        func_log(LOG_LVL_WARN, msg, args);
        va_end(args);     
} 
MAPI void mInfo(const char* msg, ...) {
        va_list args;
        va_start(args, msg);
        func_log(LOG_LVL_INFO, msg, args);
        va_end(args);
}
MAPI void mDebug(const char* msg, ...) {
        va_list args;
        va_start(args, msg);
        func_log(LOG_LVL_DEBUG, msg, args);
        va_end(args);
}
MAPI void mErr(const char* msg, ...) {
        va_list args;
        va_start(args, msg);
        func_log(LOG_LVL_ERR, msg, args);
        va_end(args);
}
MAPI void mFatal(const char* msg, ...) {
        va_list args;
        va_start(args, msg);
        func_log(LOG_LVL_FATAL, msg, args);
        va_end(args);
}