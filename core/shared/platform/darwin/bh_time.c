/*
 * Copyright (C) 2019 Intel Corporation.  All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include "bh_platform.h"

uint64
os_time_get_boot_millisecond()
{
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
        return 0;
    }

    return ((uint64) ts.tv_sec) * 1000 + ((uint64)ts.tv_nsec) / (1000 * 1000);
}

uint64
os_time_get_millisecond_from_1970()
{
    struct timeb tp;
    ftime(&tp);

    return ((uint64) tp.time) * 1000 + tp.millitm
            - (tp.dstflag == 0 ? 0 : 60 * 60 * 1000)
            + ((uint64)tp.timezone) * 60 * 1000;
}

size_t
os_time_strftime(char *s, size_t max, const char *format, uint64 time)
{
    time_t time_sec = (time_t)(time / 1000);
    struct timeb tp;
    struct tm *ltp;

    ftime(&tp);
    time_sec -= tp.timezone * 60;

    ltp = localtime(&time_sec);
    if (ltp == NULL) {
        return 0;
    }
    return strftime(s, max, format, ltp);
}

