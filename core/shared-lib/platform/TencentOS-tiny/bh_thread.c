/*
 * Copyright (C) 2019 Intel Corporation.  All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include "bh_thread.h"
#include "bh_assert.h"
#include "bh_log.h"
#include "bh_memory.h"
#include <stdio.h>
#include <stdlib.h>

#include "cmsis_os.h"

typedef struct bh_thread_wait_node {
    korp_sem sem;
    bh_thread_wait_list next;
} bh_thread_wait_node;

typedef struct bh_thread_data {
    /* Next thread data */
    struct bh_thread_data *next;
    /* Zephyr thread handle */
    korp_tid tid;
    /* Jeff thread local root */
    void *tlr;
    /* Lock for waiting list */
    korp_mutex wait_list_lock;
    /* Waiting list of other threads who are joining this thread */
    bh_thread_wait_list thread_wait_list;
    /* Thread stack size */
    unsigned stack_size;
    /* Thread stack */
    char stack[1];
} bh_thread_data;

typedef struct bh_thread_obj {
    korp_thread thread;
    /* Whether the thread is terminated and this thread object is to
     be freed in the future. */
    bool to_be_freed;
    struct bh_thread_obj *next;
} bh_thread_obj;

static bool is_thread_sys_inited = false;

/* Thread data of supervisor thread */
static bh_thread_data supervisor_thread_data;

/* Lock for thread data list */
static k_mutex_t thread_data_lock;
/* Thread data list */
static bh_thread_data *thread_data_list = NULL;
/* Lock for thread object list */
static k_mutex_t thread_obj_lock;

int _vm_thread_sys_init()
{
    if (is_thread_sys_inited)
        return BHT_OK;

    tos_mutex_create(&thread_data_lock);
    tos_mutex_create(&thread_obj_lock);

    /* Initialize supervisor thread data */
    memset(&supervisor_thread_data, 0, sizeof(supervisor_thread_data));
    supervisor_thread_data.tid = osThreadGetId();
    /* Set as head of thread data list */
    thread_data_list = &supervisor_thread_data;

    is_thread_sys_inited = true;
    return BHT_OK;
}

void vm_thread_sys_destroy(void)
{
    if (is_thread_sys_inited) {
        is_thread_sys_inited = false;
    }
}

int _vm_thread_create(korp_tid *p_tid, thread_start_routine_t start, void *arg,
                      unsigned int stack_size)
{
    return _vm_thread_create_with_prio(p_tid, start, arg, stack_size,
                                       BH_THREAD_DEFAULT_PRIORITY);
}

int _vm_thread_create_with_prio(korp_tid *p_tid, thread_start_routine_t start,
                                void *arg, unsigned int stack_size, int prio)
{
    return BHT_ERROR;
    // return BHT_OK;
}

korp_tid _vm_self_thread()
{
    return (korp_tid) osThreadGetId();
}

void vm_thread_exit(void * code)
{
    (void) code;
    korp_tid self = vm_self_thread();
    osThreadTerminate((osThreadId) self);
}

int _vm_thread_cancel(korp_tid thread)
{
    osThreadTerminate((osThreadId) thread);
    return 0;
}

int _vm_thread_join(korp_tid thread, void **value_ptr, int mills)
{
    return 0;
}

int _vm_thread_detach(korp_tid thread)
{
    (void) thread;
    return BHT_OK;
}

void *_vm_tls_get(unsigned idx)
{
    (void) idx;
    bh_thread_data *thread_data;

    bh_assert(idx == 0);
    thread_data = thread_data_current();

    return thread_data ? thread_data->tlr : NULL;
}

int _vm_tls_put(unsigned idx, void * tls)
{
    bh_thread_data *thread_data;

    (void) idx;
    bh_assert(idx == 0);
    thread_data = thread_data_current();
    bh_assert(thread_data != NULL);

    thread_data->tlr = tls;
    return BHT_OK;
}

int _vm_mutex_init(korp_mutex *mutex)
{
    (void) mutex;
    tos_mutex_create(mutex);
    return BHT_OK;
}

int _vm_recursive_mutex_init(korp_mutex *mutex)
{
    tos_mutex_create(mutex);
    return BHT_OK;
}

int _vm_mutex_destroy(korp_mutex *mutex)
{
    (void) mutex;
    return BHT_OK;
}

void vm_mutex_lock(korp_mutex *mutex)
{
    tos_mutex_pend_timed(mutex, TOS_TIME_FOREVER);
}

int vm_mutex_trylock(korp_mutex *mutex)
{
    return tos_mutex_pend_timed(mutex, TOS_TIME_NOWAIT);
}

void vm_mutex_unlock(korp_mutex *mutex)
{
    tos_mutex_post(mutex);
}

int _vm_sem_init(korp_sem* sem, unsigned int c)
{
    tos_sem_create(sem, c);
    return BHT_OK;
}

int _vm_sem_destroy(korp_sem *sem)
{
    (void) sem;
    return BHT_OK;
}

int _vm_sem_wait(korp_sem *sem)
{
    return tos_sem_pend(sem, TOS_TIME_FOREVER);
}

int _vm_sem_reltimedwait(korp_sem *sem, int mills)
{
    return tos_sem_pend(sem, mills);
}

int _vm_sem_post(korp_sem *sem)
{
    tos_sem_post(sem);
    return BHT_OK;
}

int _vm_cond_init(korp_cond *cond)
{
    _vm_mutex_init(&cond->wait_list_lock);
    cond->thread_wait_list = NULL;
    return BHT_OK;
}

int _vm_cond_destroy(korp_cond *cond)
{
    (void) cond;
    return BHT_OK;
}

static int _vm_cond_wait_internal(korp_cond *cond, korp_mutex *mutex,
        bool timed, int mills)
{
    return BHT_OK;
    //return BHT_ERROR;
}

int _vm_cond_wait(korp_cond *cond, korp_mutex *mutex)
{
    return BHT_OK;
    //return BHT_ERROR;
}

int _vm_cond_reltimedwait(korp_cond *cond, korp_mutex *mutex, int mills)
{
    return BHT_OK;
    //return BHT_ERROR;
}

int _vm_cond_signal(korp_cond *cond)
{
    return BHT_OK;
    //return BHT_ERROR;
}

int _vm_cond_broadcast(korp_cond *cond)
{
    return BHT_OK;
    //return BHT_ERROR;
}

