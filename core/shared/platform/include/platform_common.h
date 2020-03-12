/*
 * Copyright (C) 2019 Intel Corporation.  All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#ifndef _PLATFORM_COMMON_H
#define _PLATFORM_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../../../config.h"
#include "platform_internal.h"

#define BH_MAX_THREAD 32

#define BHT_ERROR (-1)
#define BHT_TIMED_OUT (1)
#define BHT_OK (0)

#define BHT_NO_WAIT 0x00000000
#define BHT_WAIT_FOREVER 0xFFFFFFFF

#define BH_KB (1024)
#define BH_MB ((BH_KB)*1024)
#define BH_GB ((BH_MB)*1024)

#ifndef BH_MALLOC
#define BH_MALLOC os_malloc
#endif

#ifndef BH_FREE
#define BH_FREE os_free
#endif

/**
 * WA_MALLOC/WA_FREE are macros called by app-framework's
 * shared source code for native and wasm app.
 * They are defined as malloc/free by default for wasm app,
 * and if they ared called by native, they should be re-defined
 * to wasm_runtime_malloc/free.
 */

#ifndef WA_MALLOC
#include <stdlib.h>
#define WA_MALLOC malloc
#endif

#ifndef WA_FREE
#include <stdlib.h>
#define WA_FREE free
#endif

#ifndef NULL
#define NULL (void*)0
#endif

#ifndef __cplusplus
#define true 1
#define false 0
#define inline __inline
#endif

/* Return the offset of the given field in the given type */
#ifndef offsetof
#define offsetof(Type, field) ((size_t)(&((Type *)0)->field))
#endif

typedef uint8_t uint8;
typedef int8_t int8;
typedef uint16_t uint16;
typedef int16_t int16;
typedef uint32_t uint32;
typedef int32_t int32;
typedef float float32;
typedef double float64;
typedef uint64_t uint64;
typedef int64_t int64;

void *wasm_runtime_malloc(unsigned int size);
void wasm_runtime_free(void *ptr);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _PLATFORM_COMMON_H */
