/*
 * Copyright (C) 2019 Intel Corporation.  All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include "bh_platform.h"

uint64
os_time_get_boot_millisecond()
{
    return k_uptime_get_32();
}

uint64
os_time_get_millisecond_from_1970()
{
    return k_uptime_get();
}

size_t
os_time_strftime(char *str, size_t max, const char *format, uint64 time)
{
    (void) format;
    uint32 t = (uint32)time;
    uint32 h, m, s;

    t = t % (24 * 60 * 60);
    h = t / (60 * 60);
    t = t % (60 * 60);
    m = t / 60;
    s = t % 60;

    return snprintf(str, max, "%02u:%02u:%02u", h, m, s);
}

