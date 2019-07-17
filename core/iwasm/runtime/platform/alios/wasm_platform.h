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

#ifndef _WASM_PLATFORM_H
#define _WASM_PLATFORM_H

#include "bh_platform.h"


#ifdef __cplusplus
extern "C" {
#endif

int wasm_platform_init();

void*
wasm_dlsym(void *handle, const char *symbol);

#ifdef __cplusplus
}
#endif

#endif
