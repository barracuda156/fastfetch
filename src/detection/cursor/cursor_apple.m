#include "cursor.h"

#import <Foundation/Foundation.h>

#include <AvailabilityMacros.h>

#if MAC_OS_X_VERSION_MIN_REQUIRED < 1090
#define POOLSTART NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
#define POOLEND   [pool release];
#else
#define POOLSTART
#define POOLEND
#endif

static void appendColor(FFstrbuf* str, NSDictionary* color)
{
    int r = (int) (((NSNumber*) [color valueForKey:@"red"]).doubleValue * 255);
    int g = (int) (((NSNumber*) [color valueForKey:@"green"]).doubleValue * 255);
    int b = (int) (((NSNumber*) [color valueForKey:@"blue"]).doubleValue * 255);
    int a = (int) (((NSNumber*) [color valueForKey:@"alpha"]).doubleValue * 255);

    if (r == 255 && g == 255 && b == 255 && a == 255)
        ffStrbufAppendS(str, "White");
    else if (r == 0 && g == 0 && b == 0 && a == 255)
        ffStrbufAppendS(str, "Black");
    else
        ffStrbufAppendF(str, "#%02X%02X%02X%02X", r, g, b, a);
}

void ffDetectCursor(FFCursorResult* result)
{
    POOLSTART
    NSDictionary* dict = [NSDictionary dictionaryWithContentsOfFile:[NSHomeDirectory() stringByAppendingPathComponent:@"/Library/Preferences/com.apple.universalaccess.plist"]];

    NSDictionary* color;

    ffStrbufAppendS(&result->theme, "Fill - ");
    if ((color = [dict valueForKey:@"cursorFill"]))
        appendColor(&result->theme, color);
    else
        ffStrbufAppendS(&result->theme, "Black");

    ffStrbufAppendS(&result->theme, ", Outline - ");

    if ((color = [dict valueForKey:@"cursorOutline"]))
        appendColor(&result->theme, color);
    else
        ffStrbufAppendS(&result->theme, "White");

    NSNumber* mouseDriverCursorSize = [dict valueForKey:@"mouseDriverCursorSize"];
    if (mouseDriverCursorSize)
        ffStrbufAppendF(&result->size, "%d", (int) (mouseDriverCursorSize.doubleValue * 32));
    else
        ffStrbufAppendS(&result->size, "32");
    POOLEND
}
