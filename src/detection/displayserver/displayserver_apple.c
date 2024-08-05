#include "displayserver.h"
#include "util/apple/cf_helpers.h"
#include "common/sysctl.h"

#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <ApplicationServices/ApplicationServices.h>
#include <AvailabilityMacros.h>

typedef union
{
    uint8_t rawData[0xDC];
    struct
    {
        uint32_t mode;
        uint32_t flags;		// 0x4
        uint32_t width;		// 0x8
        uint32_t height;	// 0xC
        uint32_t depth;		// 0x10
        uint32_t dc2[42];
        uint16_t dc3;
        uint16_t freq;		// 0xBC
        uint32_t dc4[4];
        float density;		// 0xD0
    } derived;
} modes_D4;

void CGSGetCurrentDisplayMode(CGDirectDisplayID display, int* modeNum);
void CGSGetDisplayModeDescriptionOfLength(CGDirectDisplayID display, int idx, modes_D4* mode, int length);

static void detectDisplays(FFDisplayServerResult* ds)
{
    CGDisplayCount screenCount;
    CGGetOnlineDisplayList(INT_MAX, NULL, &screenCount);
    if(screenCount == 0)
        return;

    CGDirectDisplayID* screens = malloc(screenCount * sizeof(CGDirectDisplayID));
    CGGetOnlineDisplayList(INT_MAX, screens, &screenCount);

    for(uint32_t i = 0; i < screenCount; i++)
    {
        int modeID;
        CGSGetCurrentDisplayMode(screens[i], &modeID);
        modes_D4 mode;
        CGSGetDisplayModeDescriptionOfLength(screens[i], modeID, &mode, 0xD4);

        double refreshRate = ffdsParseRefreshRate(mode.derived.freq);
        ffdsAppendDisplay(ds,
            mode.derived.width,
            mode.derived.height,
            refreshRate,
            0,
            0,
            0,
            NULL,
            FF_DISPLAY_TYPE_UNKNOWN,
            true,
            0
        );
    }

    free(screens);
}

void ffConnectDisplayServerImpl(FFDisplayServerResult* ds)
{
    {
#if MAC_OS_X_VERSION_MIN_REQUIRED >= 1080
        FF_CFTYPE_AUTO_RELEASE CFMachPortRef port = CGWindowServerCreateServerPort();
#else
        FF_CFTYPE_AUTO_RELEASE CFMachPortRef port = CGWindowServerCFMachPort();
#endif
        if (port)
        {
            ffStrbufSetStatic(&ds->wmProcessName, "WindowServer");
            ffStrbufSetStatic(&ds->wmPrettyName, "Quartz Compositor");
        }
    }
    ffStrbufSetStatic(&ds->dePrettyName, "Aqua");

    detectDisplays(ds);
}
