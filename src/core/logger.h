#pragma once

typedef enum {
        LOG_LVL_FATAL = 0,
        LOG_LVL_ERR = 1,
        LOG_LVL_WARN = 2,
        LOG_LVL_INFO = 3,
        LOG_LVL_DEBUG = 4
} logLevel;

void func_log(logLevel lvl, const char* msg);

#define mWarn(msg) func_log(LOG_LVL_WARN, msg);
#define mInfo(msg) func_log(LOG_LVL_INFO, msg);
#define mDebug(msg) func_log(LOG_LVL_DEBUG, msg);
#define mErr(msg) func_log(LOG_LVL_ERR, msg);
#define mFatal(msg) func_log(LOG_LVL_FATAL, msg);