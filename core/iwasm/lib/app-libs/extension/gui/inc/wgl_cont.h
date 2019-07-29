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

#ifndef WAMR_GRAPHIC_LIBRARY_CONT_H
#define WAMR_GRAPHIC_LIBRARY_CONT_H

#ifdef __cplusplus
extern "C" {
#endif


/** Container layout options*/
enum {
    WGL_LAYOUT_OFF = 0, /**< No layout */
    WGL_LAYOUT_CENTER, /**< Center objects */
    WGL_LAYOUT_COL_L,  /**< Column left align*/
    WGL_LAYOUT_COL_M,  /**< Column middle align*/
    WGL_LAYOUT_COL_R,  /**< Column right align*/
    WGL_LAYOUT_ROW_T,  /**< Row top align*/
    WGL_LAYOUT_ROW_M,  /**< Row middle align*/
    WGL_LAYOUT_ROW_B,  /**< Row bottom align*/
    WGL_LAYOUT_PRETTY, /**< Put as many object as possible in row and begin a new row*/
    WGL_LAYOUT_GRID,   /**< Align same-sized object into a grid*/
    _WGL_LAYOUT_NUM
};
typedef uint8_t wgl_layout_t;

/**
 * How to resize the container around the children.
 */
enum {
    WGL_FIT_NONE,  /**< Do not change the size automatically*/
    WGL_FIT_TIGHT, /**< Shrink wrap around the children */
    WGL_FIT_FLOOD, /**< Align the size to the parent's edge*/
    WGL_FIT_FILL,  /**< Align the size to the parent's edge first but if there is an object out of it
                     then get larger */
    _WGL_FIT_NUM
};
typedef uint8_t wgl_fit_t;

/*Styles*/
enum {
    WGL_CONT_STYLE_MAIN,
};
typedef uint8_t wgl_cont_style_t;


#ifdef __cplusplus
}
#endif

#endif /* WAMR_GRAPHIC_LIBRARY_CONT_H */
