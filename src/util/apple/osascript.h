#pragma once

#include "fastfetch.h"

bool ffOsascript(const char* input, FFstrbuf* result);

#include <AvailabilityMacros.h>

#if MAC_OS_X_VERSION_MIN_REQUIRED < 1090
#define POOLSTART NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
#define POOLEND   [pool release];
#else
#define POOLSTART
#define POOLEND
#endif
