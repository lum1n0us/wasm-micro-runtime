/*
 * INTEL CONFIDENTIAL
 *
 * Copyright 2017-2019 Intel Corporation
 *
 * This software and the related documents are Intel copyrighted materials,
 * and your use of them is governed by the express license under which they
 * were provided to you (License). Unless the License provides otherwise, you
 * may not use, modify, copy, publish, distribute, disclose or transmit this
 * software or the related documents without Intel's prior written permission.
 *
 * This software and the related documents are provided as is, with no express
 * or implied warranties, other than those that are expressly stated in the
 * License.
 */

#ifndef _MODULE_CONFIG_H_
#define _MODULE_CONFIG_H_

#define ENABLE_MODULE_JEFF 0
#define ENABLE_MODULE_WASM_APP 1
#define ENABLE_MODULE_WASM_LIB 1

#ifdef ENABLE_MODULE_JEFF
#include "module_jeff.h"
#endif
#ifdef ENABLE_MODULE_WASM_APP
#include "module_wasm_app.h"
#endif
#ifdef ENABLE_MODULE_WASM_LIB
#include "module_wasm_lib.h"
#endif

#endif /* _MODULE_CONFIG_H_ */
