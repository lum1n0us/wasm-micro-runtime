/*
 * Copyright (C) 2019 Intel Corporation.  All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include "bh_platform.h"

uint64
os_time_get_boot_millisecond()
{
    return (uint64)aos_now_ms();
}

uint64
os_time_get_millisecond_from_1970()
{
    return (uint64)aos_now_ms();
}

size_t
os_time_strftime(char *str, size_t max, const char *format, uint64 time)
{
    str = aos_now_time_str(str, max);
    return str ? strlen(str) + 1 : 0;
}

