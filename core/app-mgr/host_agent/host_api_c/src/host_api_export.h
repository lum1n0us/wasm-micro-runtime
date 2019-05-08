#include <stdio.h>
#include <stdlib.h>
#include "attr-container.h"

#define COAP_GET 1
#define COAP_POST 2
#define COAP_PUT 3
#define COAP_DELETE 4
#define PAYLOAD_FORMAT_ATTRIBUTE_OBJECT 1
#define PAYLOAD_FORMAT_JSON 2
#define PAYLOAD_FMT PAYLOAD_FORMAT_ATTRIBUTE_OBJECT

typedef struct aee_request_t {
    int mid;
    char *url;
    char action;
    char *payload;
} aee_request_t;

typedef struct aee_response_t {
    int mid;
    int status;
    int fmt;
    char *payload;
} aee_response_t;

typedef struct aee_applet_info_t {
    char name[256];
} aee_applet_info_t;

typedef struct aee_applet_list_t {
    int cnt;
    aee_applet_info_t *applets;
} aee_applet_list_t;

typedef void (*aee_event_listener_t)(const char *url, void *event, int fmt);

typedef void (*aee_response_handler_t)(void *usr_ctx, aee_response_t *response);
#define EXPORT_C extern "C" __declspec(dllexport)

typedef bool (*func_hostclient_initialize)(const char *ip, unsigned short port);
typedef void (*func_hostclient_shutdown)();
typedef bool (*func_hostclient_register_event)(const char *url,
        aee_event_listener_t listener);
typedef bool (*func_hostclient_unregister_event)(const char *url);
typedef void (*func_aee_request_init)(aee_request_t *req, char *url,
        int action);
typedef void (*func_aee_request_set_payload)(aee_request_t *req, void *payload,
        int size, int fmt);
typedef bool (*func_aee_request_send)(aee_request_t *request,
        aee_response_handler_t handler, void *usr_ctx);
typedef void *(*func_aee_request_get_payload)(aee_request_t *request, int *size,
        int *fmt);
typedef bool (*func_aee_applet_install)(char *buf, char* module_type,
        int length, char *applet_name, int time_out);
typedef bool (*func_aee_applet_uninstall)(char *applet_name, char* module_type,
        int time_out);
typedef bool (*func_aee_applet_list)(int time_out,
        aee_applet_list_t *applet_list);
typedef void (*func_aee_applet_list_init)(aee_applet_list_t *applet_list);
typedef void (*func_aee_applet_list_clean)(aee_applet_list_t *applet_list);
typedef const char *(*func_aee_applet_last_error)();
typedef int (*func_aee_response_get_status)(aee_response_t *response);
typedef void *(*func_aee_response_get_payload)(aee_response_t *response,
        int *size, int *fmt);
