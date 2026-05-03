#include <core/logger.h>

int main() {
        mWarn("This is a warning log message.");
        mInfo("This is an info log message.");
        mDebug("This is a debug log message.");
        mErr("This is an error log message.");
        mFatal("This is a fatal log message.");
}