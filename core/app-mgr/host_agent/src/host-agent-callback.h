/*
 * INTEL CONFIDENTIAL
 *
 * Copyright 2017-2018 Intel Corporation
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

#ifndef _HOST_AGENT_CALLBACK_H_
#define _HOST_AGENT_CALLBACK_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Foward response from aee to host client. This callback function would be
 * called once host agent receive a response from AEE.
 *
 * @param ctx_data context data for this callback
 * @param data response data from AEE
 * @param len data length
 * @param format data format
 *
 * @return success if 0, otherwise fail
 */
int
cb_foward_aee_response_to_client(void *ctx_data, void *data, int len,
        unsigned char format);

#ifdef __cplusplus
} /* end of extern "C" */
#endif

#endif /* _HOST_AGENT_CALLBACK_H_ */
