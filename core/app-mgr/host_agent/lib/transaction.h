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

#ifndef _LIB_TRANSACTION_H_
#define _LIB_TRANSACTION_H_

/*typedef*/struct sync_node_data {
    void * response;
    int response_len;
    ptr_sync_t sync_obj;
    uint8_t status;
    uint8_t response_fmt;
};

/*typedef*/struct async_node_data {
    void * context_data;
    bh_async_callback cb;
    bh_post_callback work_thread;
};

typedef struct sync_node {
    struct sync_node * next;
    //struct sync_node * prev;
    uint32_t id;
    unsigned char sync_type; // 1.sync  2. async
    uint32_t timeout;
    union {
        struct async_node_data cb_data;
        struct sync_node_data sync_data;
    };
} sync_node_t;

#endif /* _LIB_TRANSACTION_H_ */
