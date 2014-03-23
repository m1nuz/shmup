#pragma once

#if defined(_WIN32) || defined(_WIN64)

#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <windows.h>

#include "win32keys.h"

#endif

#if defined(__linux) || defined(__linux__)
#include "x11keys.h"
#endif

#if defined(__APPLE__)
#endif

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "window.h"
#include "time.h"
#include "sockets.h"
#include "thread.h"
#include "filesystem.h"

#ifdef __cplusplus
}
#endif
