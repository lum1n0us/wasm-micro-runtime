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

#ifndef _HOST_TOOL_UTILS_H_
#define _HOST_TOOL_UTILS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "attr-container.h"
#include "cJSON.h"

cJSON *attr2json(const attr_container_t *attr);

attr_container_t *json2attr(const cJSON *json);

int gen_random_id();

char *read_file_to_buffer(const char *filename, int *ret_size);

int wirte_buffer_to_file(const char *filename, const char *buffer, int size);

#ifdef __cplusplus
} /* end of extern "C" */
#endif

#endif
