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

#include "wgl.h"
#include "native_interface.h"


//wgl_obj_t wgl_page_create(wgl_obj_t par, const wgl_obj_t copy)
//{
//    return wasm_page_create(par, copy);
//}
//
//
//void wgl_page_clean(wgl_obj_t obj)
//{
//    wasm_page_clean(obj);
//}
//
//
//wgl_obj_t wgl_page_get_scrl(const wgl_obj_t page)
//{
//    return wasm_page_get_scrl(page);
//}
//
//
//uint16_t wgl_page_get_anim_time(const wgl_obj_t page)
//{
//    return wasm_page_get_anim_time(page);
//}
//
//
//void wgl_page_set_sb_mode(wgl_obj_t page, wgl_sb_mode_t sb_mode)
//{
//    wasm_page_set_sb_mode(page, sb_mode);
//}
//
//
//void wgl_page_set_anim_time(wgl_obj_t page, uint16_t anim_time)
//{
//    wasm_page_set_anim_time(page, anim_time);
//}
//
//
//void wgl_page_set_scroll_propagation(wgl_obj_t page, bool en)
//{
//    wasm_page_set_scroll_propagation(page, en);
//}
//
//
//void wgl_page_set_edge_flash(wgl_obj_t page, bool en)
//{
//    wasm_page_set_edge_flash(page, en);
//}
//
//
//void wgl_page_set_style(wgl_obj_t page, wgl_page_style_t type, const wgl_style_t * style)
//{
//    //TODO
//}
//
//
//
//wgl_sb_mode_t wgl_page_get_sb_mode(const wgl_obj_t page)
//{
//    return wasm_page_get_sb_mode(page);
//}
//
//
//bool wgl_page_get_scroll_propagation(wgl_obj_t page)
//{
//    return wasm_page_get_scroll_propagation(page);
//}
//
//
//bool wgl_page_get_edge_flash(wgl_obj_t page)
//{
//    return wasm_page_get_edge_flash(page);
//}
//
//
//wgl_coord_t wgl_page_get_fit_width(wgl_obj_t page)
//{
//    return wasm_page_get_fit_width(page);
//}
//
//
//wgl_coord_t wgl_page_get_fit_height(wgl_obj_t page)
//{
//    return wasm_page_get_fit_height(page);
//}
//
//
//
//const wgl_style_t * wgl_page_get_style(const wgl_obj_t page, wgl_page_style_t type)
//{
//    //TODO
//    return NULL;
//}
//
//
//
//
//bool wgl_page_on_edge(wgl_obj_t page, wgl_page_edge_t edge)
//{
//    return wasm_page_on_edge(page, edge);
//}
//
//
//void wgl_page_glue_obj(wgl_obj_t obj, bool glue)
//{
//    wasm_page_glue_obj(obj, glue);
//}
//
//
//void wgl_page_focus(wgl_obj_t page, const wgl_obj_t obj, wgl_anim_enable_t anim_en)
//{
//    wasm_page_focus(page, obj, anim_en);
//}
//
//
//void wgl_page_scroll_hor(wgl_obj_t page, wgl_coord_t dist)
//{
//    wasm_page_scroll_hor(page, dist);
//}
//
//
//void wgl_page_scroll_ver(wgl_obj_t page, wgl_coord_t dist)
//{
//    wasm_page_scroll_ver(page, dist);
//}
//
//
//void wgl_page_start_edge_flash(wgl_obj_t page)
//{
//    wasm_page_start_edge_flash(page);
//}
