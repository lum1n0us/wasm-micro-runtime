/*
 * Copyright (C) 2019 Intel Corporation.  All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#ifndef _PLATFORM_INTERNAL_H
#define _PLATFORM_INTERNAL_H

#include <inttypes.h>
#include <stdbool.h>
#include <assert.h>
#include <time.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdarg.h>
#include <ctype.h>
#include <pthread.h>
#include <semaphore.h>
#include <limits.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <netinet/in.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef BH_PLATFORM_LINUX
#define BH_PLATFORM_LINUX
#endif

#define _STACK_SIZE_ADJUSTMENT (32 * 1024)

/* Stack size of applet threads's native part.  */
#define BH_APPLET_PRESERVED_STACK_SIZE (8 * 1024 + _STACK_SIZE_ADJUSTMENT)

/* Default thread priority */
#define BH_THREAD_DEFAULT_PRIORITY 0

#define BH_ROUTINE_MODIFIER

#define BHT_TIMEDOUT ETIMEDOUT

#define BH_WAIT_FOREVER 0xFFFFFFFF

#define INVALID_THREAD_ID 0xFFffFFff

typedef pthread_t korp_tid;
typedef pthread_mutex_t korp_mutex;
typedef sem_t korp_sem;
typedef pthread_cond_t korp_cond;
typedef pthread_t korp_thread;
typedef void* (*thread_start_routine_t)(void*);

#define os_printf printf
#define os_vprintf vprintf
#define os_abort abort

#ifdef __cplusplus
}
#endif

#endif /* end of _PLATFORM_INTERNAL_H */

