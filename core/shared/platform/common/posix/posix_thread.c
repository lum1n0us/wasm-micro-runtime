/*
 * Copyright (C) 2019 Intel Corporation.  All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include "bh_platform.h"

typedef struct {
    thread_start_routine_t start;
    void* stack;
    uint32 stack_size;
    void* arg;
} thread_wrapper_arg;

static void *os_thread_wrapper(void *arg)
{
    thread_wrapper_arg * targ = arg;
    LOG_VERBOSE("THREAD CREATE 0x%08x\n", &targ);
    targ->stack = (void *)((uintptr_t)(&arg) & (uintptr_t)~0xfff);
    targ->start(targ->arg);
    BH_FREE(targ);
    return NULL;
}

int os_thread_create_with_prio(korp_tid *tid, thread_start_routine_t start,
                               void *arg, unsigned int stack_size, int prio)
{
    pthread_attr_t tattr;
    thread_wrapper_arg *targ;

    bh_assert(stack_size > 0);
    bh_assert(tid);
    bh_assert(start);

    *tid = INVALID_THREAD_ID;

    pthread_attr_init(&tattr);
    pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_JOINABLE);
    if (pthread_attr_setstacksize(&tattr, stack_size) != 0) {
        LOG_ERROR("Invalid thread stack size %u. Min stack size on Linux = %u",
                  stack_size, PTHREAD_STACK_MIN);
        pthread_attr_destroy(&tattr);
        return BHT_ERROR;
    }

    targ = (thread_wrapper_arg*) BH_MALLOC(sizeof(*targ));
    if (!targ) {
        pthread_attr_destroy(&tattr);
        return BHT_ERROR;
    }

    targ->start = start;
    targ->arg = arg;
    targ->stack_size = stack_size;

    if (pthread_create(tid, &tattr, os_thread_wrapper, targ) != 0) {
        pthread_attr_destroy(&tattr);
        BH_FREE(targ);
        return BHT_ERROR;
    }

    pthread_attr_destroy(&tattr);
    return BHT_OK;
}

int os_thread_create(korp_tid *tid, thread_start_routine_t start, void *arg,
                     unsigned int stack_size)
{
    return os_thread_create_with_prio(tid, start, arg, stack_size,
                                      BH_THREAD_DEFAULT_PRIORITY);
}

korp_tid os_self_thread()
{
    return (korp_tid) pthread_self();
}

int os_mutex_init(korp_mutex *mutex)
{
    return pthread_mutex_init(mutex, NULL) == 0 ? BHT_OK : BHT_ERROR;
}

int os_recursive_mutex_init(korp_mutex *mutex)
{
    int ret;

    pthread_mutexattr_t mattr;

    bh_assert(mutex);
    ret = pthread_mutexattr_init(&mattr);
    if (ret)
        return BHT_ERROR;

    pthread_mutexattr_settype(&mattr, PTHREAD_MUTEX_RECURSIVE);
    ret = pthread_mutex_init(mutex, &mattr);
    pthread_mutexattr_destroy(&mattr);

    return ret == 0 ? BHT_OK : BHT_ERROR;
}

int os_mutex_destroy(korp_mutex *mutex)
{
    int ret;

    bh_assert(mutex);
    ret = pthread_mutex_destroy(mutex);

    return ret == 0 ? BHT_OK : BHT_ERROR;
}

/* Returned error (EINVAL, EAGAIN and EDEADLK) from
 locking the mutex indicates some logic error present in
 the program somewhere.
 Don't try to recover error for an existing unknown error.*/
void os_mutex_lock(korp_mutex *mutex)
{
    int ret;

    bh_assert(mutex);
    ret = pthread_mutex_lock(mutex);
    if (0 != ret) {
        printf("vm mutex lock failed (ret=%d)!\n", ret);
        exit(-1);
    }
}

/* Returned error (EINVAL, EAGAIN and EPERM) from
 unlocking the mutex indicates some logic error present
 in the program somewhere.
 Don't try to recover error for an existing unknown error.*/
void os_mutex_unlock(korp_mutex *mutex)
{
    int ret;

    bh_assert(mutex);
    ret = pthread_mutex_unlock(mutex);
    if (0 != ret) {
        printf("vm mutex unlock failed (ret=%d)!\n", ret);
        exit(-1);
    }
}

int os_sem_init(korp_sem* sem, unsigned int c)
{
    int ret;

    bh_assert(sem);
    ret = sem_init(sem, 0, c);

    return ret == 0 ? BHT_OK : BHT_ERROR;
}

int os_sem_destroy(korp_sem *sem)
{
    int ret;

    bh_assert(sem);
    ret = sem_destroy(sem);

    return ret == 0 ? BHT_OK : BHT_ERROR;
}

int os_sem_wait(korp_sem *sem)
{
    int ret;

    bh_assert(sem);

    ret = sem_wait(sem);

    return ret == 0 ? BHT_OK : BHT_ERROR;
}

int os_sem_reltimedwait(korp_sem *sem, int mills)
{
    int ret = BHT_OK;

    struct timespec timeout;
    const int mills_per_sec = 1000;
    const int mills_to_nsec = 1E6;

    bh_assert(sem);

    if (mills == (int)BHT_WAIT_FOREVER) {
        ret = sem_wait(sem);
    } else {

        timeout.tv_sec = mills / mills_per_sec;
        timeout.tv_nsec = (mills % mills_per_sec) * mills_to_nsec;
        timeout.tv_sec += time(NULL);

        ret = sem_timedwait(sem, &timeout);
    }

    if (ret != BHT_OK) {
        if (errno == BHT_TIMEDOUT) {
            ret = BHT_TIMEDOUT;
            errno = 0;
        } else {
            LOG_ERROR("Faliure happens when timed wait is called");
            bh_assert(0);
        }
    }

    return ret;
}

int os_sem_post(korp_sem *sem)
{
    bh_assert(sem);

    return sem_post(sem) == 0 ? BHT_OK : BHT_ERROR;
}

int os_cond_init(korp_cond *cond)
{
    bh_assert(cond);

    if (pthread_cond_init(cond, NULL) != BHT_OK)
        return BHT_ERROR;

    return BHT_OK;
}

int os_cond_destroy(korp_cond *cond)
{
    bh_assert(cond);

    if (pthread_cond_destroy(cond) != BHT_OK)
        return BHT_ERROR;

    return BHT_OK;
}

int os_cond_wait(korp_cond *cond, korp_mutex *mutex)
{
    bh_assert(cond);
    bh_assert(mutex);

    if (pthread_cond_wait(cond, mutex) != BHT_OK)
        return BHT_ERROR;

    return BHT_OK;
}

static void msec_nsec_to_abstime(struct timespec *ts, int usec)
{
    struct timeval tv;

    gettimeofday(&tv, NULL);

    ts->tv_sec = (long int)(tv.tv_sec + usec / 1000000);
    ts->tv_nsec = (long int)(tv.tv_usec * 1000 + (usec % 1000000) * 1000);

    if (ts->tv_nsec >= 1000000000L) {
        ts->tv_sec++;
        ts->tv_nsec -= 1000000000L;
    }
}

int os_cond_reltimedwait(korp_cond *cond, korp_mutex *mutex, int useconds)
{
    int ret;
    struct timespec abstime;

    if (useconds == (int)BHT_WAIT_FOREVER)
        ret = pthread_cond_wait(cond, mutex);
    else {
        msec_nsec_to_abstime(&abstime, useconds);
        ret = pthread_cond_timedwait(cond, mutex, &abstime);
    }

    if (ret != BHT_OK && ret != BHT_TIMEDOUT)
        return BHT_ERROR;

    return BHT_OK;
}

int os_cond_signal(korp_cond *cond)
{
    bh_assert(cond);

    if (pthread_cond_signal(cond) != BHT_OK)
        return BHT_ERROR;

    return BHT_OK;
}

int os_thread_join(korp_tid thread, void **value_ptr)
{
    return pthread_join(thread, value_ptr);
}

