/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include "test_helper.h"
#include "gtest/gtest.h"

#include "platform_common.h"
#include "wasm_runtime_common.h"
#include "bh_read_file.h"
#include "wasm_runtime.h"
#include "bh_platform.h"
#include "wasm_export.h"
#include "aot_runtime.h"

#include "wasm_exec_env.h"

namespace {
/* Minimal WASM module that just returns 42
 * Generated with: echo '(module (func (export "test") (result i32) i32.const
 * 42))' | wat2wasm -
 */
static uint8 wasm_for_native_stack_overflow[] = {
    0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x01, 0x05,
    0x01, 0x60, 0x00, 0x01, 0x7f, 0x03, 0x02, 0x01, 0x00, 0x07,
    0x08, 0x01, 0x04, 0x74, 0x65, 0x73, 0x74, 0x00, 0x00, 0x0a,
    0x06, 0x01, 0x04, 0x00, 0x41, 0x2a, 0x0b
};
}

class wasm_exec_env_test_suite : public testing::Test
{
  protected:
    // You should make the members protected s.t. they can be
    // accessed from sub-classes.

    // virtual void SetUp() will be called before each test is run.  You
    // should define it if you need to initialize the variables.
    // Otherwise, this can be skipped.
    virtual void SetUp()
    {
        RuntimeInitArgs init_args;

        memset(&init_args, 0, sizeof(RuntimeInitArgs));

        init_args.mem_alloc_type = Alloc_With_Pool;
        init_args.mem_alloc_option.pool.heap_buf = global_heap_buf;
        init_args.mem_alloc_option.pool.heap_size = sizeof(global_heap_buf);

        ASSERT_EQ(wasm_runtime_full_init(&init_args), true);
    }

    // virtual void TearDown() will be called after each test is run.
    // You should define it if there is cleanup work to do.  Otherwise,
    // you don't have to provide it.
    //
    virtual void TearDown() { wasm_runtime_destroy(); }

    bool load_wasm_from_buf(uint8 *wasm_file_buf, uint32 wasm_file_size)
    {
        if (!(module = wasm_runtime_load(wasm_file_buf, wasm_file_size,
                                         error_buf, sizeof(error_buf)))) {
            printf("Load wasm module failed. error: %s\n", error_buf);
            goto fail;
        }
        return true;

    fail:
        if (module)
            wasm_runtime_unload(module);

        return false;
    }

    wasm_module_t module = NULL;
    char error_buf[128];
    char global_heap_buf[512 * 1024];
};

TEST_F(wasm_exec_env_test_suite, wasm_exec_env_create)
{
    uint32 stack_size = 8192;
    load_wasm_from_buf(wasm_for_native_stack_overflow,
                       sizeof(wasm_for_native_stack_overflow));

    wasm_module_inst_t inst = wasm_runtime_instantiate(module, stack_size, 0, error_buf,
                                    sizeof(error_buf));
    ASSERT_TRUE(inst != nullptr);

    /* 0 -> 1*/
    EXPECT_NE(nullptr, wasm_exec_env_create(inst, 0));

    wasm_runtime_deinstantiate(inst);
    wasm_runtime_unload(module);
}

TEST_F(wasm_exec_env_test_suite, wasm_exec_env_create_internal)
{
    uint32 stack_size = 8192;
    load_wasm_from_buf(wasm_for_native_stack_overflow,
                       sizeof(wasm_for_native_stack_overflow));

    wasm_module_inst_t inst = wasm_runtime_instantiate(module, stack_size, 0, error_buf,
                                    sizeof(error_buf));
    ASSERT_TRUE(inst != nullptr);

    EXPECT_EQ(nullptr, wasm_exec_env_create_internal(nullptr, UINT32_MAX));

    wasm_runtime_deinstantiate(inst);
    wasm_runtime_unload(module);
}

TEST_F(wasm_exec_env_test_suite, wasm_exec_env_pop_jmpbuf)
{
    WASMExecEnv exec_env;

    exec_env.jmpbuf_stack_top = nullptr;
    EXPECT_EQ(nullptr, wasm_exec_env_pop_jmpbuf(&exec_env));
}

TEST_F(wasm_exec_env_test_suite, sequential_calling_wasm)
{
    uint32 stack_size = 8192;
    wasm_module_inst_t inst = nullptr;
    wasm_exec_env_t exec_env_a = nullptr;
    wasm_exec_env_t exec_env_b = nullptr;
    wasm_function_inst_t func = nullptr;
    uint32_t argv[1];
    const char *exception = nullptr;
    /* keep me here */
    uint8_t *high_boundary;

    ASSERT_TRUE(load_wasm_from_buf(wasm_for_native_stack_overflow,
                               sizeof(wasm_for_native_stack_overflow)));

    /* Simulate handleRequest: grab instance, execute, destroy */
    int i = 0;
    for (; i < 100; i++) {
        /* Step 1: Create exec_env_A */
        inst = wasm_runtime_instantiate(module, stack_size, 0, error_buf,
                                        sizeof(error_buf));
        ASSERT_TRUE(inst != nullptr);

        exec_env_a = wasm_exec_env_create(inst, stack_size);
        ASSERT_TRUE(exec_env_a != nullptr);

        /* Step 2: Call WASM */
        argv[0] = 0;
        func = wasm_runtime_lookup_function(inst, "test");
        ASSERT_TRUE(wasm_runtime_call_wasm(exec_env_a, func, 0, argv));
        ASSERT_EQ((uint32_t)42, argv[0]);

        /* Step 3: Destroy exec_env_A */
        wasm_runtime_destroy_exec_env(exec_env_a);

        /* in case reuse previous freed blocks*/
        void *placeholder1 = wasm_runtime_malloc(sizeof(wasm_exec_env_t));
        void *placeholder2 = wasm_runtime_malloc(sizeof(wasm_exec_env_t));

        /* Step 4: Create exec_env_B*/
        exec_env_b = wasm_exec_env_create(inst, stack_size);
        ASSERT_TRUE(exec_env_b != nullptr);

        /* Step 5: Call WASM - should success */
        ASSERT_TRUE(wasm_runtime_call_wasm(exec_env_b, func, 0, argv));
        ASSERT_EQ((uint32_t)42, argv[0]);

        /* Step 6: Destroy exec_env_B and inst */
        wasm_runtime_destroy_exec_env(exec_env_b);
        wasm_runtime_deinstantiate(inst);

        wasm_runtime_free(placeholder1);
        wasm_runtime_free(placeholder2);
    }

    ASSERT_TRUE(i == 100);
    wasm_runtime_unload(module);
    module = NULL;

}

TEST_F(wasm_exec_env_test_suite, native_stack_overflow_early_return)
{
    uint32 stack_size = 8192;
    wasm_module_inst_t inst = nullptr;
    wasm_exec_env_t exec_env_a = nullptr;
    wasm_exec_env_t exec_env_b = nullptr;
    wasm_function_inst_t func = nullptr;
    uint32_t argv[1];
    const char *exception = nullptr;
    /* keep me here */
    uint8_t *high_boundary;

    ASSERT_TRUE(load_wasm_from_buf(wasm_for_native_stack_overflow,
                               sizeof(wasm_for_native_stack_overflow)));

    /* Step 1: Create exec_env_A */
    inst = wasm_runtime_instantiate(module, stack_size, 0, error_buf,
                                    sizeof(error_buf));
    ASSERT_TRUE(inst != nullptr);

    exec_env_a = wasm_exec_env_create(inst, stack_size);
    ASSERT_TRUE(exec_env_a != nullptr);

    /* Step 2: Set native_stack_boundary to trigger overflow check failure */
    high_boundary = (uint8_t *)((uintptr_t)&high_boundary + 0x100000);
    wasm_runtime_set_native_stack_boundary(exec_env_a, high_boundary);

    /* Step 3: Call WASM - should fail with native stack overflow */
    func = wasm_runtime_lookup_function(inst, "test");
    argv[0] = 0;
    ASSERT_FALSE(wasm_runtime_call_wasm(exec_env_a, func, 0, argv));

    exception = wasm_runtime_get_exception(inst);
    ASSERT_TRUE(exception != nullptr);
    ASSERT_TRUE(strstr(exception, "native stack overflow") != nullptr);
    /* Clear exception for next operations */
    wasm_runtime_clear_exception(inst);

    /* Step 4: Destroy exec_env_A */
    wasm_runtime_destroy_exec_env(exec_env_a);

    /* in case reuse previous freed blocks*/
    void *placeholder1 = wasm_runtime_malloc(sizeof(wasm_exec_env_t));
    void *placeholder2 = wasm_runtime_malloc(sizeof(wasm_exec_env_t));

    /* Step 5: Create exec_env_B*/
    exec_env_b = wasm_exec_env_create(inst, stack_size);
    ASSERT_TRUE(exec_env_b != nullptr);

    /* Step 6: Call WASM - should success */
    ASSERT_TRUE(wasm_runtime_call_wasm(exec_env_b, func, 0, argv));
    ASSERT_EQ((uint32_t)42, argv[0]);

    /* Step 7: Destroy exec_env_B and inst */
    wasm_runtime_destroy_exec_env(exec_env_b);
    wasm_runtime_deinstantiate(inst);

    wasm_runtime_free(placeholder1);
    wasm_runtime_free(placeholder2);
    wasm_runtime_unload(module);
    module = NULL;
}