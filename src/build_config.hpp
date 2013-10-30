// this file should be force-included in all translation units
#pragma once

#if defined _WIN32
#define _WIN32_WINNT 0x0601 // Windows 7 and never
#endif

static const auto MaxThreads = 4;
