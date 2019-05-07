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

#include <stdio.h>
#include <stdlib.h>

void on_init()
{
#if 0
    attr_container_t *attr;

    request_set_handler(NULL, NULL);
    response_get_status(NULL);
    response_create(NULL, 0);
    response_send(NULL);
    response_get_payload(NULL);
    sensor_open(NULL, 0, NULL, NULL);
    sensor_config(NULL, 0, 0, 0);
    sensor_config_with_attr_container(NULL, NULL);
    sensor_close(NULL);

    api_timer_create(0, 0, 0, NULL, NULL);
    api_timer_set_interval(NULL, 0);
    api_timer_cancel(NULL);
    timer_start(NULL);

    attr = attr_container_create("attr");
    attr_container_destroy(attr);
#endif
}
