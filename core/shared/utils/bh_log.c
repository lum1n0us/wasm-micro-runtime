/*
 * Copyright (C) 2019 Intel Corporation.  All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include "bh_log.h"

/**
 * The verbose level of the log system.  Only those verbose logs whose
 * levels are less than or equal to this value are outputed.
 */
static uint32 log_verbose_level = LOG_LEVEL_WARNING;

void
bh_log_set_verbose_level(uint32 level)
{
    log_verbose_level = level;
}

void
bh_log(LogLevel log_level, const char *file, int line, const char *fmt, ...)
{
    va_list ap;
    korp_tid self;
    char buf[32] = { 0 };

    if (log_level > log_verbose_level)
        return;

    self = os_self_thread();

    os_time_strftime(buf, 32, "%Y-%m-%d %H:%M:%S",
                     (int64)os_time_get_millisecond_from_1970());

    os_printf("[%s - %X]: ", buf, (uint32)self);

    if (file)
        os_printf("%s, line %d, ", file, line);

    va_start(ap, fmt);
    os_vprintf(fmt, ap);
    va_end(ap);

    os_printf("\n");
}
