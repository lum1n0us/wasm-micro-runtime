/*
 * Copyright (C) 2019 Intel Corporation.  All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include "coap_ext.h"

char * coap_get_full_url_alloc(coap_packet_t * request)
{
    char *url_alloc, *p;
    const char *url = NULL;
    const char * query = NULL;
    uint32 url_len = (uint32)coap_get_header_uri_path(request, &url);
    uint32 query_len = (uint32)coap_get_header_uri_query(request, &query);
    uint64 total_size = (uint64)url_len + 1 + (uint64)query_len + 1;

    if (url_len == 0)
        return NULL;

    if (total_size >= UINT32_MAX
        || !(url_alloc = p = (char*)bh_malloc((uint32)total_size)))
        return NULL;

    bh_memcpy_s(p, (uint32)total_size, url, url_len);
    p += url_len;
    *p = '\0';

    /* make the url looks like /abc?e=f */
    if (query_len != 0) {
        *p++ = '&';
        bh_memcpy_s(p, query_len + 1, query, query_len);
        p += query_len;
        *p = '\0';
    }

    return url_alloc;
}

void convert_request_to_coap_packet(request_t * req, coap_packet_t * packet)
{
    coap_init_message(packet, COAP_TYPE_NON, req->action, req->mid);
    coap_set_token(packet, (uint8_t *) &req->mid, sizeof(req->mid));
    coap_set_header_content_format(packet, req->fmt);

    coap_set_header_uri_path(packet, req->url);

    coap_set_payload(packet, req->payload, req->payload_len);

    packet->mid = req->mid;
}

void convert_response_to_coap_packet(response_t * response,
        coap_packet_t * packet)
{
    coap_init_message(packet, COAP_TYPE_NON, response->status, response->mid);
    coap_set_token(packet, (uint8_t *) &response->mid, sizeof(response->mid));
    coap_set_header_content_format(packet, response->fmt);
    coap_set_payload(packet, response->payload, response->payload_len);

    packet->mid = response->mid;
}

// return: the length of url.
//         note: the url is probably not end with 0 due to coap packing design.
int convert_coap_packet_to_request(coap_packet_t *packet, request_t *request)
{
    const char *url = NULL;
    int url_len = coap_get_header_uri_path(packet, &url);

    memset(request, 0, sizeof(*request));

    request->action = packet->code;
    request->fmt = packet->content_format;
    if (packet->token_len == 4) {
        request->mid = *((unsigned long *) packet->token);
    } else {
        request->mid = packet->mid;
    }
    request->payload = packet->payload;
    request->payload_len = packet->payload_len;
    request->url = (char *)url;
    return url_len;
}

void convert_coap_packet_to_response(coap_packet_t *packet,
        response_t *response)
{
    memset(response, 0, sizeof(*response));

    response->status = packet->code;
    response->fmt = packet->content_format;
    if (packet->token_len == 4) {
        response->mid = *((unsigned long *) packet->token);
    } else {
        response->mid = packet->mid;
    }

    response->payload = packet->payload;
    response->payload_len = packet->payload_len;
    return;
}
