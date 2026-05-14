#include <mschne.h>

int main() {
        mWarn("This won't show when passing MSUPPRESS.");
        mInfo("This won't show when passing MSUPPRESS.");
        mDebug("This won't show when passing MSUPPRESS.");
        mErr("This won't show when passing MSUPPRESS.");
        mFatal("This will show even when passing MSUPPRESS.");
}