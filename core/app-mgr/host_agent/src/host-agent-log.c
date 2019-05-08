/*
 * INTEL CONFIDENTIAL
 *
 * Copyright 2017-2018 Intel Corporation
 *
 * This software and the related documents are Intel copyrighted materials,
 * and your use of them is governed by the express license under which they
 * were provided to you (License). Unless the License provides otherwise, you
 * may not use, modify, copy, publish, distribute, disclose or transmit this
 * software or the related documents without Intel's prior written permission.
 *
 * This software and the related documents are provided as is, with no express
 * or implied warranties, other than those that are expressly stated in the
 * License.
 */
/**
 * @file   host-agent-log.c
 *
 * @brief Log system.
 */

#define _GNU_SOURCE

#include "host-agent-log.h"
#include <stdio.h>
#include <pthread.h>

/**
 * The verbose level of the log system.  Only those verbose logs whose
 * levels are less than or equal to this value are outputed.
 */
static int log_verbose_level;

/**
 * The lock for protecting the global output stream of logs.
 */
static pthread_mutex_t log_stream_lock;

static FILE *log_file = NULL;

void log_init(int level, const char* path)
{
    pthread_mutexattr_t mattr;
    FILE *file;

    log_verbose_level = level;

    pthread_mutexattr_init(&mattr);
    pthread_mutexattr_settype(&mattr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&log_stream_lock, &mattr);
    pthread_mutexattr_destroy(&mattr);

    if (path && (file = fopen(path, "a+"))) {
        log_file = file;
        fseek(log_file, 0, SEEK_END);
    } else {
        log_file = stdout;
    }
}

void log_destroy()
{
    pthread_mutex_destroy(&log_stream_lock);
    if (log_file != stdout)
        fclose(log_file);
}

static void log_vprintf(const char *fmt, va_list ap)
{
    pthread_mutex_lock(&log_stream_lock);
    vfprintf(log_file, fmt, ap);
    fflush(log_file);
    pthread_mutex_unlock(&log_stream_lock);
}

static void log_printf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    log_vprintf(fmt, ap);
    va_end(ap);
}

/*
 * use this api to print log
 */
void log_record(int level, const char *tag, const char *file, int line,
        const char *fmt, ...)
{
    va_list ap;
    time_t t;
    struct tm *lt;

    pthread_t tid = pthread_self();

    if (level >= log_verbose_level)
        return;

    log_printf("[");

    t = time(NULL);
    lt = localtime(&t);
    if (lt) {
        char str[32];
        strftime(str, sizeof(str), "%Y-%m-%d %H:%M:%S", lt);
        log_printf("%s ", str);
    }

    log_printf("%s %X", PROGRAM_NAME, tid);

    if (tag)
        log_printf(" %s", tag);

    log_printf("] ");

    if (file)
        log_printf("%s:%d ", file, line);

    va_start(ap, fmt);
    log_vprintf(fmt, ap);
    va_end(ap);
}

#if 0
static void
log_set_level(int level)
{
    log_verbose_level = level;
}

int main() {
    int i = 1;

    log_init(5, "daemon_test_log.txt");

    LOG_VERBOSE("the %d log\n", i++);
    LOG_VERBOSE("the %d log\n", i++);
    log_record(2, "tag", __FILE__, __LINE__, "the %d log\n", i++);

    log_set_level(3);
    LOG_VERBOSE("the %d log\n", i++);
    LOG_ERROR("the %d log\n", i++);
}
#endif
