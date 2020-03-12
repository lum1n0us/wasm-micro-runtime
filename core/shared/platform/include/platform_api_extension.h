/*
 * Copyright (C) 2019 Intel Corporation.  All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#ifndef PLATFORM_API_EXTENSION_H
#define PLATFORM_API_EXTENSION_H

#include "platform_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Extension interfaces to support more features, e.g. multi-thread
 * The related data structures should be defined in platform_internal.h
 */

/**
 * Ceates a thread
 *
 * @param p_tid  [OUTPUT] the pointer of tid
 * @param start  main routine of the thread
 * @param arg  argument passed to main routine
 * @param stack_size  bytes of stack size
 *
 * @return 0 if success.
 */
int os_thread_create(korp_tid *p_tid, thread_start_routine_t start, void *arg,
                     unsigned int stack_size);

/**
 * Creates a thread with priority
 *
 * @param p_tid  [OUTPUT] the pointer of tid
 * @param start  main routine of the thread
 * @param arg  argument passed to main routine
 * @param stack_size  bytes of stack size
 * @param prio the priority
 *
 * @return 0 if success.
 */
int os_thread_create_with_prio(korp_tid *p_tid, thread_start_routine_t start,
                               void *arg, unsigned int stack_size, int prio);

/**
 * Waits for the thread specified by thread to terminate
 *
 * @param thread the thread to wait
 * @param retval if not NULL, output the exit status of the terminated thread
 *
 * @return return 0 if success
 */
int os_thread_join(korp_tid thread, void **retval);

/**
 * Suspend execution of the calling thread for (at least)
 * usec microseconds
 *
 * @param return 0 if success, -1 otherwise
 */
int os_usleep(uint32 usec);

/**
 * This function creates a condition variable
 *
 * @param cond [OUTPUT] pointer to condition variable
 *
 * @return 0 if success
 */
int os_cond_init(korp_cond *cond);

/**
 * This function destroys condition variable
 *
 * @param cond pointer to condition variable
 *
 * @return 0 if success
 */
int os_cond_destroy(korp_cond *cond);

/**
 * Wait a condition varible.
 *
 * @param cond pointer to condition variable
 * @param mutex pointer to mutex to protect the condition variable
 *
 * @return 0 if success
 */
int os_cond_wait(korp_cond *cond, korp_mutex *mutex);

/**
 * Wait a condition varible or return if time specified passes.
 *
 * @param cond pointer to condition variable
 * @param mutex pointer to mutex to protect the condition variable
 * @param mills milliseconds to wait
 *
 * @return 0 if success
 */
int os_cond_reltimedwait(korp_cond *cond, korp_mutex *mutex, int mills);

/**
 * Signals the condition variable
 *
 * @param cond condition variable
 *
 * @return 0 if success
 */
int os_cond_signal(korp_cond *cond);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef PLATFORM_API_EXTENSION_H */
