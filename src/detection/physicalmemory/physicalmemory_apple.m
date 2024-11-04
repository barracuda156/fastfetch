#include "physicalmemory.h"
#include "common/processing.h"
#include "util/smbiosHelper.h"
#include "util/stringUtils.h"

#include <Foundation/Foundation.h>
#include <AvailabilityMacros.h>

#if MAC_OS_X_VERSION_MIN_REQUIRED < 1090
#define POOLSTART NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
#define POOLEND   [pool release];
#else
#define POOLSTART
#define POOLEND
#endif

static void appendDevice(
    FFlist* result,
    NSString* type,
    NSString* vendor,
    NSString* size,

    // Intel and PowerPC
    NSString* locator,
    NSString* serial,
    NSString* partNumber,
    NSString* speed,
    bool ecc)
{
    FFPhysicalMemoryResult* device = ffListAdd(result);
    ffStrbufInitS(&device->type, type.UTF8String);
    ffStrbufInit(&device->formFactor);
    ffStrbufInitS(&device->locator, locator.UTF8String);
    ffStrbufInitS(&device->vendor, vendor.UTF8String);
    FFPhysicalMemoryUpdateVendorString(device);
    ffStrbufInitS(&device->serial, serial.UTF8String);
    ffCleanUpSmbiosValue(&device->serial);
    ffStrbufInitS(&device->partNumber, partNumber.UTF8String);
    ffCleanUpSmbiosValue(&device->partNumber);
    device->size = 0;
    device->maxSpeed = 0;
    device->runningSpeed = 0;
    device->ecc = ecc;

    if (size)
    {
        char* unit = NULL;
        device->size = strtoul(size.UTF8String, &unit, 10);
        if (*unit == ' ') ++unit;

        switch (*unit)
        {
            case 'G': device->size *= 1024ULL * 1024 * 1024; break;
            case 'M': device->size *= 1024ULL * 1024; break;
            case 'K': device->size *= 1024ULL; break;
            case 'T': device->size *= 1024ULL * 1024 * 1024 * 1024; break;
        }
    }

    if (speed)
    {
        char* unit = NULL;
        device->maxSpeed = (uint32_t) strtoul(speed.UTF8String, &unit, 10);
        if (*unit == ' ') ++unit;

        switch (*unit)
        {
            case 'T': device->maxSpeed *= 1000 * 1000; break;
            case 'G': device->maxSpeed *= 1000; break;
            case 'K': device->maxSpeed /= 1000; break;
        }
        device->runningSpeed = device->maxSpeed;
    }
}

const char* ffDetectPhysicalMemory(FFlist* result)
{
    POOLSTART
    FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();
    if (ffProcessAppendStdOut(&buffer, (char* const[]) {
        "system_profiler",
        "SPMemoryDataType",
        "-xml",
        "-detailLevel",
        "full",
        NULL
    }) != NULL)
        return "Starting `system_profiler SPMemoryDataType -xml -detailLevel full` failed";

    NSString* error;
    NSData* plistData = [NSData dataWithBytes:buffer.chars length:buffer.length];
    NSPropertyListFormat format;
    NSArray* arr = [NSPropertyListSerialization propertyListFromData:plistData
                    mutabilityOption:NSPropertyListImmutable
                    format:&format
                    errorDescription:&error];
    if (!arr) {
        return "system_profiler SPMemoryDataType returned an empty array";
        [error release];
    }

    for (NSDictionary* data in [[arr objectAtIndex:0] objectForKey:@"_items"])
    {
        if ([data objectForKey:@"_items"])
        {
            // for Intel and PowerPC
            for (NSDictionary* item in [data objectForKey:@"_items"])
            {
                appendDevice(result,
                    [item valueForKey:@"dimm_type"],
                    [item valueForKey:@"dimm_manufacturer"],
                    [item valueForKey:@"dimm_size"],
                    [item valueForKey:@"_name"],
                    [item valueForKey:@"dimm_serial_number"],
                    [item valueForKey:@"dimm_part_number"],
                    [item valueForKey:@"dimm_speed"],
                    !![[data objectForKey:@"global_ecc_state"] isEqualToString:@"ecc_enabled"]);
            }
        }
#ifdef __arm64__
        else
        {
            // for Apple Silicon
            appendDevice(result,
                data[@"dimm_type"],
                data[@"dimm_manufacturer"],
                data[@"SPMemoryDataType"],
                nil,
                nil,
                nil,
                nil,
                false);
        }
#endif
    }
    POOLEND
    return NULL;
}
