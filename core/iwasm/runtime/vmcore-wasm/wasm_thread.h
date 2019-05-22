/*
 * Copyright (C) 2019 Intel Corporation.  All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _WASM_RUNTIME_THREAD_H
#define _WASM_RUNTIME_THREAD_H

#include "wasm_assert.h"
#include "wa_thread.h"

#ifdef __cplusplus
extern "C" {
#endif

struct WASMModuleInstance;
struct WASMInterpFrame;

typedef struct WASMStack {
    /* The bottom of the stack, must be 8-bytes align. */
    uint8 *bottom;
    /* Top cell index which is free. */
    uint8 *top;
    /* The top boundary of the stack. */
    uint8 *top_boundary;
} WASMStack;

#if WASM_ENABLE_HASH_BLOCK_ADDR == 0
typedef struct BlockAddr {
    const uint8 *start_addr;
    uint8 *else_addr;
    uint8 *end_addr;
} BlockAddr;

#define BLOCK_ADDR_CACHE_SIZE 256
#endif

typedef struct WASMThread {
    /* Previous thread's tlr of an instance. */
    struct WASMThread *prev;

    /* Next thread's tlr of an instance. */
    struct WASMThread *next;

    /* The WASM module instance of current thread */
    struct WASMModuleInstance *module_inst;

    /* Current frame of current thread. */
    struct WASMInterpFrame *cur_frame;

    /* The boundary of native stack. When interpreter detects that native
       frame may overrun this boundary, it will throw a stack overflow
       exception. */
    void *native_stack_boundary;

    /* The WASM stack of current thread. */
    WASMStack wasm_stack;

    /* The native thread handle of current thread. */
    korp_tid handle;

    /* Current suspend count of this thread.  */
    uint32 suspend_count;

#if WASM_ENABLE_HASH_BLOCK_ADDR == 0
    /* Cache the block address if hashmap is disabled. */
    BlockAddr block_addr_cache[BLOCK_ADDR_CACHE_SIZE];
#endif
} WASMThread;

/**
 * Allocate a WASM frame from the WASM stack.
 *
 * @param tlr the current thread
 * @param size size of the WASM frame, it must be a multiple of 4
 *
 * @return the WASM frame if there is enough space in the stack area
 * with a protection area, NULL otherwise
 */
static inline void*
wasm_thread_alloc_wasm_frame(WASMThread *tlr, unsigned size)
{
    uint8 *addr = tlr->wasm_stack.top;

    wasm_assert(!(size & 3));

    /* The outs area size cannot be larger than the frame size, so
       multiplying by 2 is enough. */
    if (addr + size * 2 > tlr->wasm_stack.top_boundary) {
        /* WASM stack overflow. */
        /* When throwing SOE, the preserved space must be enough. */
        /*wasm_assert(!tlr->throwing_soe);*/
        return NULL;
    }

    tlr->wasm_stack.top += size;

    return addr;
}

static inline void
wasm_thread_free_wasm_frame(WASMThread *tlr, void *prev_top)
{
    wasm_assert((uint8 *)prev_top >= tlr->wasm_stack.bottom);
    tlr->wasm_stack.top = (uint8 *)prev_top;
}

/**
 * Get the current WASM stack top pointer.
 *
 * @param tlr the current thread
 *
 * @return the current WASM stack top pointer
 */
static inline void*
wasm_thread_wasm_stack_top(WASMThread *tlr)
{
    return tlr->wasm_stack.top;
}

/**
 * Set the current frame pointer.
 *
 * @param tlr the current thread
 * @param frame the WASM frame to be set for the current thread
 */
static inline void
wasm_thread_set_cur_frame(WASMThread *tlr, struct WASMInterpFrame *frame)
{
    tlr->cur_frame = frame;
}

/**
 * Get the current frame pointer.
 *
 * @param tlr the current thread
 *
 * @return the current frame pointer
 */
static inline struct WASMInterpFrame*
wasm_thread_get_cur_frame(WASMThread *tlr)
{
    return tlr->cur_frame;
}

#ifdef __cplusplus
}
#endif

#endif /* end of _WASM_RUNTIME_THREAD_H */
