#pragma once 
// API Header for MSCHNE

#include "globals.h"
#include "core/platform/platform_common.h"
#include "core/logger.h"
#include "core/render.h"

// Platform

MAPI b8 m_createWin(app_window win);

// Logger

MAPI void mWarn(const char* msg);
MAPI void mInfo(const char* msg);
MAPI void mDebug(const char* msg);
MAPI void mErr(const char* msg);
MAPI void mFatal(const char* msg);

// Render

MAPI void RenderWindow();