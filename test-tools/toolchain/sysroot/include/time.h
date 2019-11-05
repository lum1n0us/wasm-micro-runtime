/*
 * Copyright (C) 2019 Intel Corporation.  All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#ifndef _WAMR_LIBC_TIME_H
#define _WAMR_LIBC_TIME_H

#ifdef __cplusplus
extern "C" {
#endif

typedef long time_t;

time_t time(time_t *t);

#ifdef __cplusplus
}
#endif

#endif