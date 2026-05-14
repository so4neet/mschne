#include <mschne.h>

const char *string = "String!";
int num = 24;

int main() {
        mWarn("This is a warning log message.");
        mInfo("This is an info log message.");
        mDebug("This is a debug log message.");
        mErr("This is an error log message.");
        mFatal("This is a fatal log message.");
        mWarn("You can also pass through variables: %s %i", string, num);
}