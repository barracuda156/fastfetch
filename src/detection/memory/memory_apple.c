#include "memory.h"

#include <string.h>
#include <mach/mach.h>
#include <sys/sysctl.h>
#include <unistd.h>
#include <AvailabilityMacros.h>

const char* ffDetectMemory(FFMemoryResult* ram)
{
    size_t length = sizeof(ram->bytesTotal);
    if (sysctl((int[]){ CTL_HW, HW_MEMSIZE }, 2, &ram->bytesTotal, &length, NULL, 0))
        return "Failed to read hw.memsize";

#if defined(__i386__) || defined(__ppc__)
    mach_msg_type_number_t count = HOST_VM_INFO_COUNT;
    vm_statistics_data_t vmstat;
    if(host_statistics(mach_host_self(), HOST_VM_INFO, (host_info_t) (&vmstat), &count) != KERN_SUCCESS)
        return "Failed to read host_statistics";
#else
    mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;
    vm_statistics64_data_t vmstat;
    if(host_statistics64(mach_host_self(), HOST_VM_INFO64, (host_info64_t) (&vmstat), &count) != KERN_SUCCESS)
        return "Failed to read host_statistics64";
#endif

    // https://github.com/exelban/stats/blob/master/Modules/RAM/readers.swift#L56
    ram->bytesUsed = ((uint64_t)
        + vmstat.active_count
        + vmstat.inactive_count
        + vmstat.speculative_count
        + vmstat.wire_count
#if (MAC_OS_X_VERSION_MIN_REQUIRED >= 1060) && !defined(__ppc__)
        + vmstat.compressor_page_count
        - vmstat.purgeable_count
        - vmstat.external_page_count
#endif
    ) * instance.state.platform.sysinfo.pageSize;

    return NULL;
}
