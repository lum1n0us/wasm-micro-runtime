/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include "thread_test_common.h"

/* One fixture: the table index given to spawn, and whether the routine at
   table index 0 has the signature that pthread_create must accept */
struct SpawnCase
{
    const char *fixture;
    uint32 elem_index;
    bool accepted;
};

class PthreadSpawnTest : public ThreadModuleFixture,
                         public testing::WithParamInterface<SpawnCase>
{};

TEST_P(PthreadSpawnTest, SpawnChecksRoutineSignature)
{
    SpawnCase c = GetParam();

    ASSERT_TRUE(Instantiate(c.fixture));

    int32 ret = CallI32("spawn", { c.elem_index });
    if (c.accepted) {
        /* The routine returns its argument, join must succeed */
        EXPECT_EQ(ret, 0);
        EXPECT_EQ(CallI32("join", {}), 0);
    }
    else {
        EXPECT_EQ(ret, -1);
    }
}

INSTANTIATE_TEST_SUITE_P(
    Signature, PthreadSpawnTest,
    testing::Values(
        SpawnCase{ "routine_valid", 0, true },
        SpawnCase{ "routine_no_result", 0, false },
        SpawnCase{ "routine_multi_result", 0, false },
        SpawnCase{ "routine_i64_result", 0, false },
        SpawnCase{ "routine_f32_result", 0, false },
        SpawnCase{ "routine_two_params", 0, false },
        SpawnCase{ "routine_i64_param", 0, false },
        SpawnCase{ "routine_no_params", 0, false },
        SpawnCase{ "table_out_of_range", 1, false },
        SpawnCase{ "table_null_element", 1, false }),
    [](const testing::TestParamInfo<SpawnCase> &info) {
        return std::string(info.param.fixture);
    });
