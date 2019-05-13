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

#ifndef MOUSEWHEEL_H
#define MOUSEWHEEL_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#ifdef LV_CONF_INCLUDE_SIMPLE
#include "lv_drv_conf.h"
#else
#include "../../lv_drv_conf.h"
#endif

#if USE_MOUSEWHEEL

#include <stdbool.h>
#include "lvgl/lv_hal/lv_hal_indev.h"

#ifndef MONITOR_SDL_INCLUDE_PATH
#define MONITOR_SDL_INCLUDE_PATH <SDL2/SDL.h>
#endif

#include MONITOR_SDL_INCLUDE_PATH

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Initialize the encoder
 */
void mousewheel_init(void);

/**
 * Get the mouse wheel position change.
 * @param data store the read data here
 * @return false: all ticks and button state are handled
 */
bool mousewheel_read(lv_indev_data_t * data);

/**
 * It is called periodically from the SDL thread to check a key is pressed/released
 * @param event describes the event
 */
void mousewheel_handler(SDL_Event *event);

/**********************
 *      MACROS
 **********************/

#endif /*USE_MOUSEWHEEL*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*MOUSEWHEEL_H*/
