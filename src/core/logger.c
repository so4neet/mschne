#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "logger.h"
#include "../globals.h"

static void func_log(logLevel lvl, const char* msg, b8 isNewline) {
        const char* lvl_clr[5] = {"0;41", "1;31", "1;33", "1;32", "1;34"};
        const char* lvl_str[5] = {"[FATAL] | ",
                                  "[ERROR] | ",
                                  "[WARN] | ",
                                  "[INFO] | ",
                                  "[DEBUG] | "};
#if !defined MSUPPRESS
        if (isNewline) {
                printf("\033[%sm%s%s\033[0m\n", lvl_clr[lvl], lvl_str[lvl], msg);
        } else {
                printf("\033[%sm%s%s\033[0m", lvl_clr[lvl], lvl_str[lvl], msg);
        }

#endif
}

MAPI void mWarn(const char* msg) {
        func_log(LOG_LVL_WARN, msg, TRUE);     
} 
MAPI void mInfo(const char* msg) {
        func_log(LOG_LVL_INFO, msg, TRUE);
}
MAPI void mDebug(const char* msg) {
        func_log(LOG_LVL_DEBUG, msg, TRUE);
}
MAPI void mDebugNN(const char* msg) {
        func_log(LOG_LVL_DEBUG, msg, FALSE);
}
MAPI void mErr(const char* msg) {
        func_log(LOG_LVL_ERR, msg, TRUE);
}
MAPI void mFatal(const char* msg) {
        func_log(LOG_LVL_FATAL, msg, TRUE);
}