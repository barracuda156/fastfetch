#include "displayserver.h"

uint32_t ffdsParseRefreshRate(int32_t refreshRate)
{
    if(refreshRate <= 0)
        return 0;

    int remainder = refreshRate % 5;
    if(remainder >= 3)
        refreshRate += (5 - remainder);
    else
        refreshRate -= remainder;

    // All other typicall refresh rates are dividable by 5
    if(refreshRate == 145)
        refreshRate = 144;

    return (uint32_t) refreshRate;
}

bool ffdsAppendDisplay(
    FFDisplayServerResult* result,
    uint32_t width,
    uint32_t height,
    double refreshRate,
    uint32_t scaledWidth,
    uint32_t scaledHeight,
    uint32_t rotation,
    FFstrbuf* name,
    FFDisplayType type,
    bool primary,
    uint64_t id)
{
    if(width == 0 || height == 0)
        return false;

    FFDisplayResult* display = ffListAdd(&result->displays);
    display->width = width;
    display->height = height;
    display->refreshRate = refreshRate;
    display->scaledWidth = scaledWidth;
    display->scaledHeight = scaledHeight;
    display->rotation = rotation;
    ffStrbufInitMove(&display->name, name);
    display->type = type;
    display->primary = primary;
    display->id = id;

    return true;
}

void ffConnectDisplayServerImpl(FFDisplayServerResult* ds);

const FFDisplayServerResult* ffConnectDisplayServer()
{
    static FFDisplayServerResult result;
    if (result.displays.elementSize == 0)
    {
        ffStrbufInit(&result.wmProcessName);
        ffStrbufInit(&result.wmPrettyName);
        ffStrbufInit(&result.wmProtocolName);
        ffStrbufInit(&result.deProcessName);
        ffStrbufInit(&result.dePrettyName);
        ffListInit(&result.displays, sizeof(FFDisplayResult));
        ffConnectDisplayServerImpl(&result);
    }
    return &result;
}
