#pragma once

#include "../globals.h"

typedef enum {
        LOG_LVL_FATAL = 0,
        LOG_LVL_ERR = 1,
        LOG_LVL_WARN = 2,
        LOG_LVL_INFO = 3,
        LOG_LVL_DEBUG = 4
} logLevel;

static void func_log(logLevel lvl, const char* msg, va_list args);

MAPI void mWarn(const char* msg, ...);
MAPI void mInfo(const char* msg, ...);
MAPI void mDebug(const char* msg, ...);
MAPI void mErr(const char* msg, ...);
MAPI void mFatal(const char* msg, ...);