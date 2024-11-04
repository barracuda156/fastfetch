#include "osascript.h"

#include <Foundation/Foundation.h>
#include <AppKit/AppKit.h>
#include <CoreData/CoreData.h>

bool ffOsascript(const char* input, FFstrbuf* result) {
    POOLSTART
    NSString* appleScript = [NSString stringWithUTF8String: input];
    NSAppleScript* script = [[NSAppleScript alloc] initWithSource:appleScript];
    NSDictionary* errInfo = nil;
    NSAppleEventDescriptor* descriptor = [script executeAndReturnError:&errInfo];
    if (errInfo)
        return false;

    ffStrbufSetS(result, [[descriptor stringValue] cStringUsingEncoding:NSUTF8StringEncoding]);
    POOLEND
    return true;
}
