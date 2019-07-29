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

#ifndef WAMR_GRAPHIC_LIBRARY_H
#define WAMR_GRAPHIC_LIBRARY_H

#ifdef __cplusplus
extern "C" {
#endif

//#include "lv_conf.h"

//misc
#include "inc/wgl_types.h"

#include "wgl_shared_utils.h" /* shared types between app and native */

//objx
#include "inc/wgl_obj.h"
#include "inc/wgl_btn.h"
#include "inc/wgl_cb.h"
#include "inc/wgl_cont.h"
#include "inc/wgl_label.h"
#include "inc/wgl_list.h"

typedef int32_t wgl_obj_t;

///////////////////////////////////////
//obj
//////////////////////////////////////
typedef void (*wgl_event_cb_t)(wgl_obj_t obj, wgl_event_t event);
void wgl_obj_align(wgl_obj_t obj, wgl_obj_t base, wgl_align_t align, wgl_coord_t x_mod, wgl_coord_t y_mod);
void wgl_obj_set_event_cb(wgl_obj_t obj, wgl_event_cb_t event_cb);
wgl_res_t wgl_obj_del(wgl_obj_t obj);
void wgl_obj_del_async(wgl_obj_t obj);
void wgl_obj_clean(wgl_obj_t obj);


///////////////////////////////////////
//cont
//////////////////////////////////////

///**********************
// * GLOBAL PROTOTYPES
// **********************/
//
///**
// * Create a container objects
// * @param par pointer to an object, it will be the parent of the new container
// * @param copy pointer to a container object, if not NULL then the new object will be copied from it
// * @return pointer to the created container
// */
//wgl_obj_t wgl_cont_create(wgl_obj_t par, const wgl_obj_t copy);
//
///*=====================
// * Setter functions
// *====================*/
//
///**
// * Set a layout on a container
// * @param cont pointer to a container object
// * @param layout a layout from 'wgl_cont_layout_t'
// */
//void wgl_cont_set_layout(wgl_obj_t cont, wgl_layout_t layout);
//
///**
// * Set the fit policy in all 4 directions separately.
// * It tell how to change the container's size automatically.
// * @param cont pointer to a container object
// * @param left left fit policy from `wgl_fit_t`
// * @param right right fit policy from `wgl_fit_t`
// * @param top top fit policy from `wgl_fit_t`
// * @param bottom bottom fit policy from `wgl_fit_t`
// */
//void wgl_cont_set_fit4(wgl_obj_t cont, wgl_fit_t left, wgl_fit_t right, wgl_fit_t top, wgl_fit_t bottom);
//
///**
// * Set the fit policy horizontally and vertically separately.
// * It tells how to change the container's size automatically.
// * @param cont pointer to a container object
// * @param hor horizontal fit policy from `wgl_fit_t`
// * @param ver vertical fit policy from `wgl_fit_t`
// */
//static inline void wgl_cont_set_fit2(wgl_obj_t cont, wgl_fit_t hor, wgl_fit_t ver)
//{
//    wgl_cont_set_fit4(cont, hor, hor, ver, ver);
//}
//
///**
// * Set the fit policy in all 4 direction at once.
// * It tells how to change the container's size automatically.
// * @param cont pointer to a container object
// * @param fit fit policy from `wgl_fit_t`
// */
//static inline void wgl_cont_set_fit(wgl_obj_t cont, wgl_fit_t fit)
//{
//    wgl_cont_set_fit4(cont, fit, fit, fit, fit);
//}
//
///**
// * Set the style of a container
// * @param cont pointer to a container object
// * @param type which style should be set (can be only `WGL_CONT_STYLE_MAIN`)
// * @param style pointer to the new style
// */
//static inline void wgl_cont_set_style(wgl_obj_t cont, wgl_cont_style_t type, const wgl_style_t * style)
//{
//    (void)type; /*Unused*/
//    wgl_obj_set_style(cont, style);
//}
//
///*=====================
// * Getter functions
// *====================*/
//
///**
// * Get the layout of a container
// * @param cont pointer to container object
// * @return the layout from 'wgl_cont_layout_t'
// */
//wgl_layout_t wgl_cont_get_layout(const wgl_obj_t cont);
//
///**
// * Get left fit mode of a container
// * @param cont pointer to a container object
// * @return an element of `wgl_fit_t`
// */
//wgl_fit_t wgl_cont_get_fit_left(const wgl_obj_t cont);
//
///**
// * Get right fit mode of a container
// * @param cont pointer to a container object
// * @return an element of `wgl_fit_t`
// */
//wgl_fit_t wgl_cont_get_fit_right(const wgl_obj_t cont);
//
///**
// * Get top fit mode of a container
// * @param cont pointer to a container object
// * @return an element of `wgl_fit_t`
// */
//wgl_fit_t wgl_cont_get_fit_top(const wgl_obj_t cont);
//
///**
// * Get bottom fit mode of a container
// * @param cont pointer to a container object
// * @return an element of `wgl_fit_t`
// */
//wgl_fit_t wgl_cont_get_fit_bottom(const wgl_obj_t cont);
//
///**
// * Get the style of a container
// * @param cont pointer to a container object
// * @param type which style should be get (can be only `WGL_CONT_STYLE_MAIN`)
// * @return pointer to the container's style
// */
//static inline const wgl_style_t * wgl_cont_get_style(const wgl_obj_t cont, wgl_cont_style_t type)
//{
//    (void)type; /*Unused*/
//    return wgl_obj_get_style(cont);
//}



///////////////////////////////////////
//btn
//////////////////////////////////////
wgl_obj_t wgl_btn_create(wgl_obj_t par, wgl_obj_t copy);

/*=====================
 * Setter functions
 *====================*/

/**
 * Enable the toggled states. On release the button will change from/to toggled state.
 * @param btn pointer to a button object
 * @param tgl true: enable toggled states, false: disable
 */
void wgl_btn_set_toggle(wgl_obj_t btn, bool tgl);

/**
 * Set the state of the button
 * @param btn pointer to a button object
 * @param state the new state of the button (from wgl_btn_state_t enum)
 */
void wgl_btn_set_state(wgl_obj_t btn, wgl_btn_state_t state);

/**
 * Toggle the state of the button (ON->OFF, OFF->ON)
 * @param btn pointer to a button object
 */
void wgl_btn_toggle(wgl_obj_t btn);

///**
// * Set the layout on a button
// * @param btn pointer to a button object
// * @param layout a layout from 'wgl_cont_layout_t'
// */
//static inline void wgl_btn_set_layout(wgl_obj_t btn, wgl_layout_t layout)
//{
//    wgl_cont_set_layout(btn, layout);
//}
//
///**
// * Set the fit policy in all 4 directions separately.
// * It tells how to change the button size automatically.
// * @param btn pointer to a button object
// * @param left left fit policy from `wgl_fit_t`
// * @param right right fit policy from `wgl_fit_t`
// * @param top top fit policy from `wgl_fit_t`
// * @param bottom bottom fit policy from `wgl_fit_t`
// */
//static inline void wgl_btn_set_fit4(wgl_obj_t btn, wgl_fit_t left, wgl_fit_t right, wgl_fit_t top, wgl_fit_t bottom)
//{
//    wgl_cont_set_fit4(btn, left, right, top, bottom);
//}
//
///**
// * Set the fit policy horizontally and vertically separately.
// * It tells how to change the button size automatically.
// * @param btn pointer to a button object
// * @param hor horizontal fit policy from `wgl_fit_t`
// * @param ver vertical fit policy from `wgl_fit_t`
// */
//static inline void wgl_btn_set_fit2(wgl_obj_t btn, wgl_fit_t hor, wgl_fit_t ver)
//{
//    wgl_cont_set_fit2(btn, hor, ver);
//}
//
///**
// * Set the fit policy in all 4 direction at once.
// * It tells how to change the button size automatically.
// * @param btn pointer to a button object
// * @param fit fit policy from `wgl_fit_t`
// */
//static inline void wgl_btn_set_fit(wgl_obj_t btn, wgl_fit_t fit)
//{
//    wgl_cont_set_fit(btn, fit);
//}

/**
 * Set time of the ink effect (draw a circle on click to animate in the new state)
 * @param btn pointer to a button object
 * @param time the time of the ink animation
 */
void wgl_btn_set_ink_in_time(wgl_obj_t btn, uint16_t time);

/**
 * Set the wait time before the ink disappears
 * @param btn pointer to a button object
 * @param time the time of the ink animation
 */
void wgl_btn_set_ink_wait_time(wgl_obj_t btn, uint16_t time);

/**
 * Set time of the ink out effect (animate to the released state)
 * @param btn pointer to a button object
 * @param time the time of the ink animation
 */
void wgl_btn_set_ink_out_time(wgl_obj_t btn, uint16_t time);

/**
 * Set a style of a button.
 * @param btn pointer to button object
 * @param type which style should be set
 * @param style pointer to a style
 *  */
void wgl_btn_set_style(wgl_obj_t btn, wgl_btn_style_t type, const wgl_style_t * style);

/*=====================
 * Getter functions
 *====================*/

/**
 * Get the current state of the button
 * @param btn pointer to a button object
 * @return the state of the button (from wgl_btn_state_t enum)
 */
wgl_btn_state_t wgl_btn_get_state(const wgl_obj_t btn);

/**
 * Get the toggle enable attribute of the button
 * @param btn pointer to a button object
 * @return true: toggle enabled, false: disabled
 */
bool wgl_btn_get_toggle(const wgl_obj_t btn);

///**
// * Get the layout of a button
// * @param btn pointer to button object
// * @return the layout from 'wgl_cont_layout_t'
// */
//static inline wgl_layout_t wgl_btn_get_layout(const wgl_obj_t btn)
//{
//    return wgl_cont_get_layout(btn);
//}
//
///**
// * Get the left fit mode
// * @param btn pointer to a button object
// * @return an element of `wgl_fit_t`
// */
//static inline wgl_fit_t wgl_btn_get_fit_left(const wgl_obj_t btn)
//{
//    return wgl_cont_get_fit_left(btn);
//}
//
///**
// * Get the right fit mode
// * @param btn pointer to a button object
// * @return an element of `wgl_fit_t`
// */
//static inline wgl_fit_t wgl_btn_get_fit_right(const wgl_obj_t btn)
//{
//    return wgl_cont_get_fit_right(btn);
//}
//
///**
// * Get the top fit mode
// * @param btn pointer to a button object
// * @return an element of `wgl_fit_t`
// */
//static inline wgl_fit_t wgl_btn_get_fit_top(const wgl_obj_t btn)
//{
//    return wgl_cont_get_fit_top(btn);
//}
//
///**
// * Get the bottom fit mode
// * @param btn pointer to a button object
// * @return an element of `wgl_fit_t`
// */
//static inline wgl_fit_t wgl_btn_get_fit_bottom(const wgl_obj_t btn)
//{
//    return wgl_cont_get_fit_bottom(btn);
//}

/**
 * Get time of the ink in effect (draw a circle on click to animate in the new state)
 * @param btn pointer to a button object
 * @return the time of the ink animation
 */
uint16_t wgl_btn_get_ink_in_time(const wgl_obj_t btn);

/**
 * Get the wait time before the ink disappears
 * @param btn pointer to a button object
 * @return the time of the ink animation
 */
uint16_t wgl_btn_get_ink_wait_time(const wgl_obj_t btn);

/**
 * Get time of the ink out effect (animate to the releases state)
 * @param btn pointer to a button object
 * @return the time of the ink animation
 */
uint16_t wgl_btn_get_ink_out_time(const wgl_obj_t btn);

/**
 * Get style of a button.
 * @param btn pointer to button object
 * @param type which style should be get
 * @return style pointer to the style
 *  */
const wgl_style_t * wgl_btn_get_style(const wgl_obj_t btn, wgl_btn_style_t type);


///////////////////////////////////////
//label
//////////////////////////////////////
wgl_obj_t wgl_label_create(wgl_obj_t par, wgl_obj_t copy);
void wgl_label_set_text(wgl_obj_t label, const char * text);




///////////////////////////////////////
//check box
//////////////////////////////////////

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create a check box objects
 * @param par pointer to an object, it will be the parent of the new check box
 * @param copy pointer to a check box object, if not NULL then the new object will be copied from it
 * @return pointer to the created check box
 */
wgl_obj_t wgl_cb_create(wgl_obj_t par, const wgl_obj_t copy);

/*=====================
 * Setter functions
 *====================*/

/**
 * Set the text of a check box. `txt` will be copied and may be deallocated
 * after this function returns.
 * @param cb pointer to a check box
 * @param txt the text of the check box. NULL to refresh with the current text.
 */
void wgl_cb_set_text(wgl_obj_t cb, const char * txt);

/**
 * Set the text of a check box. `txt` must not be deallocated during the life
 * of this checkbox.
 * @param cb pointer to a check box
 * @param txt the text of the check box. NULL to refresh with the current text.
 */
void wgl_cb_set_static_text(wgl_obj_t cb, const char * txt);


/*=====================
 * Getter functions
 *====================*/

/**
 * Get the text of a check box
 * @param cb pointer to check box object
 * @return pointer to the text of the check box
 */
const char * wgl_cb_get_text(const wgl_obj_t cb);




#ifdef __cplusplus
}
#endif

#endif /* WAMR_GRAPHIC_LIBRARY_H */
