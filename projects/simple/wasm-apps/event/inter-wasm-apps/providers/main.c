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

#include "wasm_app.h"

void publish_overheat_event()
{
    attr_container_t *event;

    event = attr_container_create("event");
    attr_container_set_string(&event, "warning", "temperature is over high");

    printf("###app publish event begin ###\n");

    api_publish_event("alert/overheat", FMT_ATTR_CONTAINER, event,
            attr_container_get_serialize_length(event));

    printf("###app publish event end ###\n");

    attr_container_destroy(event);
}

void on_init()
{
    publish_overheat_event();
}

void on_destroy()
{
    /* real destroy work including killing timer and closing sensor is accomplished in wasm app library version of on_destroy() */
}
