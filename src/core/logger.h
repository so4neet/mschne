#pragma once

typedef enum {
        LOG_LVL_FATAL = 0,
        LOG_LVL_ERR = 1,
        LOG_LVL_WARN = 2,
        LOG_LVL_INFO = 3,
        LOG_LVL_DEBUG = 4
} logLevel;

void func_log(logLevel lvl, const char* msg);

void mWarn(const char* msg);
void mInfo(const char* msg);
void mDebug(const char* msg);
void mErr(const char* msg);
void mFatal(const char* msg);