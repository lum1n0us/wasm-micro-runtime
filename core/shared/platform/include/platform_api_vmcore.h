/*
 * Copyright (C) 2019 Intel Corporation.  All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#ifndef _PLATFORM_API_VMCORE_H
#define _PLATFORM_API_VMCORE_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Section one: basic interfaces required by runtime.
 */

/**
 * Initialize platform
 *
 * @return 0 if success
 */
int bh_platform_init();

/**
 * Destroy platform
 */
void bh_platform_destroy();

/**
 * Memory related interfaces, which are used when init runtime with
 * system memory allocator, or Alloc_With_System_Allocator flag is set.
 * Return NULL or leave them empty if init runtime with other memory
 * allocators
 */
void *os_malloc(unsigned size);
void *os_realloc(void *ptr, unsigned size);
void os_free(void *ptr);

int os_printf(const char *format, ...);
int os_vprintf(const char *format, va_list ap);
int os_snprintf(const char *str, uint32 size,
                const char *format, va_list ap);
void os_abort(void);

/*
 * Get milliseconds after boot.
 *
 * @return milliseconds after boot.
 */
uint64 os_time_get_boot_millisecond(void);

/*
 * Get GMT milliseconds since from 1970.1.1, AKA UNIX time.
 * @return milliseconds since from 1970.1.1.
 */
uint64 os_time_get_millisecond_from_1970(void);

/**
 * Format the time to a string, it is only used to output log,
 * not a critical interface. If not easy to implement,
 * just return 0.
 */
size_t os_time_strftime(char *s, size_t max,
                        const char *format, uint64 time);

/**
 * Thread related interfaces, they are only required by multi-thread
 * support, if we just want to enable vmcore, we can just return 0 or
 * leave them empty.
 */

/**
 * Get current thread id, if multi-thread isn't required, it is only
 * used to output log, not a critical interface. If not easy to
 * implement, just return 0.
 *
 * @return current thread id
 */
korp_tid os_self_thread(void);

/**
 * Creates a mutex, if multi-thread isn't required,
 * just return 0
 *
 * @param mutex [OUTPUT] pointer to mutex initialized.
 *
 * @return 0 if success
 */
int os_mutex_init(korp_mutex *mutex);

/**
 * Destroys a mutex, , if multi-thread isn't required,
 * just return 0
 *
 * @param mutex pointer to mutex need destroy
 *
 * @return 0 if success
 */
int os_mutex_destroy(korp_mutex *mutex);

/**
 * Lock the mutex, , if multi-thread isn't required,
 * just return 0
 *
 * @param mutex pointer to mutex need lock
 *
 * @return Void
 */
void os_mutex_lock(korp_mutex *mutex);

/**
 * Unlock the mutex, , if multi-thread isn't required,
 * just return 0
 *
 * @param mutex pointer to mutex need unlock
 *
 * @return Void
 */
void os_mutex_unlock(korp_mutex *mutex);

/**
 * Section 2: AOT related interfaces, implement them if we want to
 * enable AOT.
 */

/* Memory map modes */
enum {
    MMAP_PROT_NONE = 0,
    MMAP_PROT_READ = 1,
    MMAP_PROT_WRITE = 2,
    MMAP_PROT_EXEC = 4
};

/* Memory map flags */
enum {
    MMAP_MAP_NONE = 0,
    /* Put the mapping into 0 to 2 G, supported only on x86_64 */
    MMAP_MAP_32BIT = 1,
    /* Don't interpret addr as a hint: place the mapping at exactly
       that address. */
    MMAP_MAP_FIXED = 2
};

void *os_mmap(void *hint, unsigned int size, int prot, int flags);
void os_munmap(void *addr, uint32 size);
int os_mprotect(void *addr, uint32 size, int prot);

/**
 * Flush cpu data cache, in some CPUs, after applying relocation to the
 * AOT code, the code may haven't been written back to the cpu data cache,
 * which may cause unexpected behaviour when executing the AOT code.
 * Implement this function if required, or just leave it empty.
 */
void os_dcache_flush(void);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _PLATFORM_API_VMCORE_H */
