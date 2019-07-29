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

#ifndef WAMR_GRAPHIC_LIBRARY_BTN_H
#define WAMR_GRAPHIC_LIBRARY_BTN_H

#ifdef __cplusplus
extern "C" {
#endif

/** Possible states of a button.
 * It can be used not only by buttons but other button-like objects too*/
enum {
    /**Released*/
    WGL_BTN_STATE_REL,

    /**Pressed*/
    WGL_BTN_STATE_PR,

    /**Toggled released*/
    WGL_BTN_STATE_TGL_REL,

    /**Toggled pressed*/
    WGL_BTN_STATE_TGL_PR,

    /**Inactive*/
    WGL_BTN_STATE_INA,

    /**Number of states*/
    _WGL_BTN_STATE_NUM,
};
typedef uint8_t wgl_btn_state_t;

/**Styles*/
enum {
    /** Release style */
    WGL_BTN_STYLE_REL,

    /**Pressed style*/
    WGL_BTN_STYLE_PR,

    /** Toggle released style*/
    WGL_BTN_STYLE_TGL_REL,

    /** Toggle pressed style */
    WGL_BTN_STYLE_TGL_PR,

    /** Inactive style*/
    WGL_BTN_STYLE_INA,
};
typedef uint8_t wgl_btn_style_t;


#ifdef __cplusplus
}
#endif

#endif /* WAMR_GRAPHIC_LIBRARY_BTN_H */
