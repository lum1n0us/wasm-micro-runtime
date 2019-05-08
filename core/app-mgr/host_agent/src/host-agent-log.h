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

#ifndef HOST_AGENT_LOG_H_
#define HOST_AGENT_LOG_H_

#include <stdarg.h>

#define PROGRAM_NAME "host-agent"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize the log system
 */
void log_init();

/**
 * Destroy the log system
 */
void log_destroy();

/**
 * Record a log
 */
void log_record(int level, const char *tag, const char *file, int line,
        const char *fmt, ...);

#ifdef __cplusplus
} /* end of extern "C" */
#endif

#define LOG_FATAL(...)      log_record(0, NULL, NULL, 0, __VA_ARGS__)
#define LOG_ERROR(...)      log_record(1, NULL, NULL, 0, __VA_ARGS__)
#define LOG_WARNING(...)    log_record(2, NULL, NULL, 0, __VA_ARGS__)
#define LOG_INFO(...)       log_record(3, NULL, NULL, 0, __VA_ARGS__)
#define LOG_VERBOSE(...)    log_record(4, NULL, NULL, 0, __VA_ARGS__)

#ifdef HOST_AGENT_DEBUG
#define LOG_DEBUG(...)      log_record(5, "DEBUG", __FILE__, __LINE__, __VA_ARGS__)
#endif

#endif /* HOST_AGENT_LOG_H_ */
