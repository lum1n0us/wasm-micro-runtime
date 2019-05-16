# Copyright (C) 2019 Intel Corporation.  All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required (VERSION 3.13.1)
message ("vgl_native_ui_app...")
project (vgl_native_ui_app)


SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -DLV_CONF_INCLUDE_SIMPLE -DPLATFORM_NATIVE_LINUX -DUSE_MONITOR -DUSE_MOUSE=1")

include(FetchContent)

set(lv_name lvgl)

set(ep_base ${PROJECT_BINARY_DIR}/ext_proj)
set_property(DIRECTORY PROPERTY "EP_BASE" ${ep_base})

set(lv_SOURCE_DIR ${ep_base}/Source/${lv_name})
set(lv_SUBBUILD_DIR ${ep_base}/Subbuild/${lv_name})

set(LVGL_DRIVER_DIR ${CMAKE_CURRENT_LIST_DIR}/lv_drivers)

FetchContent_Declare(
    ${lv_name}
    GIT_REPOSITORY https://github.com/littlevgl/lvgl.git
    GIT_TAG        v5.3
    SOURCE_DIR     ${lv_SOURCE_DIR}
    BINARY_DIR     ${lv_SOURCE_DIR}
    SUBBUILD_DIR   ${lv_SUBBUILD_DIR}
)


FetchContent_GetProperties(${lv_name})
if(NOT ${lv_name}_POPULATED)
    FetchContent_Populate(${lv_name})
endif()


set(LVGL_SOURCE_DIR ${${lv_name}_SOURCE_DIR})
message(${LVGL_SOURCE_DIR})

SET (LVGL_SOURCES
    ${LVGL_SOURCE_DIR}/lv_core/lv_group.c
    ${LVGL_SOURCE_DIR}/lv_core/lv_indev.c
    ${LVGL_SOURCE_DIR}/lv_core/lv_lang.c
    ${LVGL_SOURCE_DIR}/lv_core/lv_obj.c
    ${LVGL_SOURCE_DIR}/lv_core/lv_refr.c
    ${LVGL_SOURCE_DIR}/lv_core/lv_style.c
    ${LVGL_SOURCE_DIR}/lv_core/lv_vdb.c

    ${LVGL_SOURCE_DIR}/lv_draw/lv_draw.c
    ${LVGL_SOURCE_DIR}/lv_draw/lv_draw_arc.c
    ${LVGL_SOURCE_DIR}/lv_draw/lv_draw_img.c
    ${LVGL_SOURCE_DIR}/lv_draw/lv_draw_label.c
    ${LVGL_SOURCE_DIR}/lv_draw/lv_draw_line.c
    ${LVGL_SOURCE_DIR}/lv_draw/lv_draw_rbasic.c
    ${LVGL_SOURCE_DIR}/lv_draw/lv_draw_rect.c
    ${LVGL_SOURCE_DIR}/lv_draw/lv_draw_triangle.c
    ${LVGL_SOURCE_DIR}/lv_draw/lv_draw_vbasic.c

    ${LVGL_SOURCE_DIR}/lv_hal/lv_hal_disp.c
    ${LVGL_SOURCE_DIR}/lv_hal/lv_hal_indev.c
    ${LVGL_SOURCE_DIR}/lv_hal/lv_hal_tick.c

    ${LVGL_SOURCE_DIR}/lv_misc/lv_anim.c
    ${LVGL_SOURCE_DIR}/lv_misc/lv_area.c
    ${LVGL_SOURCE_DIR}/lv_misc/lv_circ.c
    ${LVGL_SOURCE_DIR}/lv_misc/lv_color.c
    ${LVGL_SOURCE_DIR}/lv_misc/lv_font.c
    ${LVGL_SOURCE_DIR}/lv_misc/lv_fs.c
    ${LVGL_SOURCE_DIR}/lv_misc/lv_gc.c
    ${LVGL_SOURCE_DIR}/lv_misc/lv_ll.c
    ${LVGL_SOURCE_DIR}/lv_misc/lv_log.c
    ${LVGL_SOURCE_DIR}/lv_misc/lv_math.c
    ${LVGL_SOURCE_DIR}/lv_misc/lv_mem.c
    ${LVGL_SOURCE_DIR}/lv_misc/lv_task.c
    ${LVGL_SOURCE_DIR}/lv_misc/lv_templ.c
    ${LVGL_SOURCE_DIR}/lv_misc/lv_txt.c

    ${LVGL_SOURCE_DIR}/lv_objx/lv_arc.c
    ${LVGL_SOURCE_DIR}/lv_objx/lv_bar.c
    ${LVGL_SOURCE_DIR}/lv_objx/lv_btn.c
    ${LVGL_SOURCE_DIR}/lv_objx/lv_btnm.c
    ${LVGL_SOURCE_DIR}/lv_objx/lv_calendar.c
    ${LVGL_SOURCE_DIR}/lv_objx/lv_canvas.c
    ${LVGL_SOURCE_DIR}/lv_objx/lv_cb.c
    ${LVGL_SOURCE_DIR}/lv_objx/lv_chart.c
    ${LVGL_SOURCE_DIR}/lv_objx/lv_cont.c
    ${LVGL_SOURCE_DIR}/lv_objx/lv_ddlist.c
    ${LVGL_SOURCE_DIR}/lv_objx/lv_gauge.c
    ${LVGL_SOURCE_DIR}/lv_objx/lv_img.c
    ${LVGL_SOURCE_DIR}/lv_objx/lv_imgbtn.c
    ${LVGL_SOURCE_DIR}/lv_objx/lv_kb.c
    ${LVGL_SOURCE_DIR}/lv_objx/lv_label.c
    ${LVGL_SOURCE_DIR}/lv_objx/lv_led.c
    ${LVGL_SOURCE_DIR}/lv_objx/lv_line.c
    ${LVGL_SOURCE_DIR}/lv_objx/lv_list.c
    ${LVGL_SOURCE_DIR}/lv_objx/lv_lmeter.c
    ${LVGL_SOURCE_DIR}/lv_objx/lv_mbox.c
    ${LVGL_SOURCE_DIR}/lv_objx/lv_objx_templ.c
    ${LVGL_SOURCE_DIR}/lv_objx/lv_page.c
    ${LVGL_SOURCE_DIR}/lv_objx/lv_preload.c
    ${LVGL_SOURCE_DIR}/lv_objx/lv_roller.c
    ${LVGL_SOURCE_DIR}/lv_objx/lv_slider.c
    ${LVGL_SOURCE_DIR}/lv_objx/lv_spinbox.c
    ${LVGL_SOURCE_DIR}/lv_objx/lv_sw.c
    ${LVGL_SOURCE_DIR}/lv_objx/lv_ta.c
    ${LVGL_SOURCE_DIR}/lv_objx/lv_table.c
    ${LVGL_SOURCE_DIR}/lv_objx/lv_tabview.c
    ${LVGL_SOURCE_DIR}/lv_objx/lv_tileview.c
    ${LVGL_SOURCE_DIR}/lv_objx/lv_win.c

    ${LVGL_SOURCE_DIR}/lv_themes/lv_theme.c
    ${LVGL_SOURCE_DIR}/lv_themes/lv_theme_alien.c
    ${LVGL_SOURCE_DIR}/lv_themes/lv_theme_default.c
    ${LVGL_SOURCE_DIR}/lv_themes/lv_theme_material.c
    ${LVGL_SOURCE_DIR}/lv_themes/lv_theme_mono.c
    ${LVGL_SOURCE_DIR}/lv_themes/lv_theme_nemo.c
    ${LVGL_SOURCE_DIR}/lv_themes/lv_theme_night.c
    ${LVGL_SOURCE_DIR}/lv_themes/lv_theme_templ.c
    ${LVGL_SOURCE_DIR}/lv_themes/lv_theme_zen.c

    ${LVGL_SOURCE_DIR}/lv_fonts/lv_font_builtin.c
    ${LVGL_SOURCE_DIR}/lv_fonts/lv_font_dejavu_20.c
    ${LVGL_DRIVER_DIR}/linux_display_indev.c
    ${LVGL_DRIVER_DIR}/indev/mouse.c

)
SET(SOURCES
    ${LVGL_SOURCES}
    ${CMAKE_CURRENT_LIST_DIR}/main.c
    )
include_directories(
    ${ep_base}/Source
    ${LVGL_DRIVER_DIR}
    ${LVGL_DRIVER_DIR}/display
    ${LVGL_DRIVER_DIR}/indev
    ${LVGL_SOURCE_DIR}
    ${CMAKE_CURRENT_LIST_DIR}
)
add_executable(vgl_native_ui_app ${SOURCES} )

target_link_libraries( vgl_native_ui_app -lSDL2)
