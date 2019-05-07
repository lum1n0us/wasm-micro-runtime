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

/**
 * @file   host-agent.c
 *
 * @brief Agent between host-clent and AEE.
 */

#include <libgen.h>

#include "host-agent.h"
#include "host-agent-log.h"
#include "host-agent-aee.h"
#include "host-agent-client.h"
#include "host-agent-imrt-link.h"

#define DEBUG 0

#define BUF_SIZE 4*1024

/************* globals **************/
#if defined (AEE_TCP)
int g_aee_sock = -1;
#elif defined (AEE_UART)
int g_aee_uart = -1;
#elif defined (AEE_HECI)
int g_aee_heci = -1;
#elif defined (AEE_AON)
int g_aee_aon;
#endif
host_client_t *g_host_clients = NULL;
sync_ctx_t *g_transaction_ctx = NULL;
int g_quit_thread = 0;
/***********************************/

static int host_listen_sock = -1;
static uint8_t *buffer = NULL;

extern bool offline_verify_init();

static int handle_msg(const char *buf, int len, imrt_link_recv_context_t *ctx,
        host_client_t *client)
{
    int result = -1;
    const char *pos = buf;

    LOG_INFO("handle_msg buf:%p len:%d\n", buf, len);

#if DEBUG
    int i = 0;
    for (; i < len; i++) {
        printf(" 0x%02x", buf[i]);
    }
    printf("\n");
#endif

    while (len-- > 0) {
        result = on_imrt_link_byte_arrive((unsigned char) *pos++, ctx);
        switch (result) {
        case 0:
            if (client != NULL)
                host_agent_client_handle_msg(client, &ctx->message);
            else
                host_agent_aee_handle_msg(&ctx->message);
            break;

        case 2:
            // if current in payload recieiving, do payload copy
            break;

        default:
            break;
        }
    }
    return result;
}

static void cleanup()
{
    host_agent_client_destroy();
    host_agent_aee_destroy();

    if (host_listen_sock > 0)
        close(host_listen_sock);

    if (buffer != NULL)
        free(buffer);

    log_destroy();
}

static void print_usage(const char *exe)
{
    printf("\nUsage:\n");
    printf("%s\n", exe);
    printf("\t [-log_level <1-5, default to 2>]\n");
    printf("\t [-log_file <path, default to stdout>]\n");
    printf("\t [-client_listen <port, default to 3456>]\n");
#if defined (AEE_TCP)
    printf("\t [-aee_listen <port, default to 8888>]\n");
#elif defined (AEE_UART)
    printf("\t [-aee_uart <name, default to /dev/pts/23>]\n");
#elif defined (AEE_HECI)
    //TODO
#endif
}

static bool check_path(const char *path)
{
    if (strcmp(path, "stdout") != 0) {
        char *name = realpath(path, NULL);
        if (name == NULL) {
            if (errno == ENOMEM) {
                LOG_FATAL("realpath fail as not enough memory\n");
                return false;
            } else if (errno == ENOENT) {
                char *dirc, *dname;
                dirc = strdup(path);
                if (dirc == NULL) {
                    LOG_FATAL("strdup fail\n");
                    return false;
                }
                dname = dirname(dirc);
                name = realpath(dname, NULL);
                free(dirc);
                if (name == NULL) {
                    if (errno == ENOMEM) {
                        LOG_FATAL("realpath fail as not enough memory\n");
                    } else {
                        printf("invalid log file path: %s\n", path);
                    }
                    return false;
                }
            } else {
                printf("invalid log file path: %s\n", path);
                return false;
            }
        }
        {
            bool match = false;
            static const char *dirs[] = { "/etc", "/opt", "/dev", "/usr",
                    "/lib", "/proc", "/run", "/bin", "/sbin", "/sys", "/var",
                    "/boot" };
            int i, len;
            for (i = 0; i < sizeof(dirs) / sizeof(char*); i++) {
                len = strlen(dirs[i]);
                if (strncmp(name, dirs[i], len) == 0
                        && (name[len] == '\0' || name[len] == '/')) {
                    match = true;
                    break;
                }
            }
            free(name);
            if (match) {
                printf("invalid log file path: %s\n", path);
                return false;
            }
        }
    }
    return true;
}

int main(int argc, char *argv[])
{
#ifdef AEE_TCP
    int aee_listen_sock = -1;
    bool aee_connected = false;
    uint32_t sock_errs = 0;
#endif
    int i, log_level = 2;
    char *log_file = "stdout";
#if defined (AEE_UART)
    char *aee_port = "/dev/pts/23";
#elif defined (AEE_TCP)
    char *aee_port = "8888";
#endif
    char *host_port = "3456";
    imrt_link_recv_context_t aee_message_recv_ctx = { 0 };

    /* parse options */
    for (i = 1; i < argc && argv[i][0] == '-'; i++) {
        if (strcmp(argv[i], "-log_level") == 0) {
            if (++i < argc && argv[i][0] != '-') {
                log_level = atoi(argv[i]);
                if (log_level < 1 || log_level > 5) {
                    print_usage(argv[0]);
                    return -1;
                }
            } else {
                print_usage(argv[0]);
                return -1;
            }
        } else if (strcmp(argv[i], "-log_file") == 0) {
            if (++i < argc && argv[i][0] != '-') {
                log_file = argv[i];
            } else {
                print_usage(argv[0]);
                return -1;
            }
        } else if (strcmp(argv[i], "-client_listen") == 0) {
            if (++i < argc && argv[i][0] != '-') {
                host_port = argv[i];
            } else {
                print_usage(argv[0]);
                return -1;
            }
#if defined (AEE_TCP)
        } else if (strcmp(argv[i], "-aee_listen") == 0) {
            if (++i < argc && argv[i][0] != '-') {
                aee_port = argv[i];
            } else {
                print_usage(argv[0]);
                return -1;
            }
#elif defined (AEE_UART)
        } else if (strcmp(argv[i], "-aee_uart") == 0) {
            if (++i < argc && argv[i][0] != '-') {
                aee_port = argv[i];
            } else {
                print_usage(argv[0]);
                return -1;
            }
#elif defined (AEE_HECI)
            //TODO
#endif
        } else {
            print_usage(argv[0]);
            return -1;
        }
    }

    if (!check_path(log_file)) {
        return -1;
    }

    log_init(log_level, strcmp(log_file, "stdout") ? log_file : NULL);

    /* malloc memory for buffer to recevie socket data */
    if (NULL == (buffer = (uint8_t *) malloc(BUF_SIZE))) {
        LOG_FATAL("malloc fail for socket receive buffer\n");
        goto fail;
    }

    /* listen host client connection */
    if ((host_listen_sock = host_agent_client_init(host_port)) < 0) {
        LOG_FATAL("host_agent_client_init fail: %d\n", host_listen_sock);
        goto fail;
    }
    LOG_VERBOSE("host_agent_client_init success: port %s sock %d\n", host_port,
            host_listen_sock);

#if defined (AEE_TCP)
    /* listen aee connection */
    if ((aee_listen_sock = host_agent_aee_init(aee_port)) < 0) {
        LOG_FATAL("host_agent_aee_init fail: %s\n", aee_port);
        goto fail;
    }
    LOG_VERBOSE("host_agent_aee_init success: port:%s listensock:%d\n", aee_port, aee_listen_sock);
#elif defined (AEE_UART)
    if ((g_aee_uart = host_agent_aee_init(aee_port)) < 0) {
        LOG_FATAL("host_agent_aee_init fail: %s\n", aee_port);
        goto fail;
    }
    LOG_VERBOSE("host_agent_aee_init success: port:%s uartfd:%d\n", aee_port, g_aee_uart);
#elif defined (AEE_HECI)
    // TODO:
#elif defined (AEE_AON)
    if ((g_aee_aon = host_agent_aee_init(NULL)) < 0) {
        LOG_FATAL("host_agent_aee_init fail\n");
        goto fail;
    }
    LOG_VERBOSE("host_agent_aee_init success: aon pipe read fd:%d\n", g_aee_aon);
#endif

    /* create global sync ctx */
    if (NULL == (g_transaction_ctx = create_sync_ctx())) {
        LOG_FATAL("create global sync ctx fail\n");
        goto fail;
    }

    while (g_quit_thread == 0) {
        int result = 0;
        fd_set readfds;
        struct timeval tv;
        host_client_t *client = g_host_clients;

        /* check timeout for all async transactions */
        uint32_t next_expiry_ms = bh_handle_expired_trans(g_transaction_ctx);

        LOG_VERBOSE("next_expiry_ms: %d\n", next_expiry_ms);

        if (next_expiry_ms != -1) {
            tv.tv_sec = next_expiry_ms / 1000;
            tv.tv_usec = (next_expiry_ms % 1000) * 1000;
        } else {
            tv.tv_sec = 3;
            tv.tv_usec = 0;
        }

        FD_ZERO(&readfds);

#if defined (AEE_TCP)
        if (aee_connected) {
            FD_SET(g_aee_sock, &readfds);
        } else {
            FD_SET(aee_listen_sock, &readfds);
        }
#elif defined (AEE_UART)
        FD_SET(g_aee_uart, &readfds);
#elif defined (AEE_AON)
        FD_SET(g_aee_aon, &readfds);
#elif defined (AEE_HECI)
        //TODO:
#endif

        FD_SET(host_listen_sock, &readfds);

        while (client) {
            host_client_t *next_client = client->next;
            if (client->sock >= 0) {
                LOG_VERBOSE("add client sock %d to readfds\n", client->sock);
                FD_SET(client->sock, &readfds);
            } else {
                if (client->wake_timestamp != 0
                        && client->wake_timestamp + 60 < time(NULL)) {
                    LOG_INFO("timeout after wake client: sock: %d\n",
                            client->sock);
                    host_agent_client_destroy_client(client);
                }
            }
            client = next_client;
        }

        result = select(FD_SETSIZE, &readfds, NULL, NULL, &tv);

        LOG_VERBOSE("select ret %d\n", result);

        if (result < 0) {
            if (errno != EINTR) {
                LOG_ERROR("Error in select(): %d\n", errno);
            }
        } else if (result == 0) { /* timeout */
            /* do nothing */
        } else if (result > 0) {
            int numBytes;

            if (FD_ISSET(host_listen_sock, &readfds)) {
                /* Connection request on original host client listen socket. */
                int client_sock;
                struct sockaddr_in clientname;
                size_t size = sizeof(clientname);

                client_sock = accept(host_listen_sock,
                        (struct sockaddr *) &clientname, &size);

                if (client_sock < 0) {
                    LOG_ERROR(
                            "accept host client connection fail with error:%d\n",
                            errno);
                    continue;
                }

                if (host_agent_client_add_client(client_sock) != 0) {
                    close(client_sock);
                    LOG_ERROR("add new client fail\n");
                    continue;
                }

                LOG_INFO("connect from host client %s, port %d, sock %d\n",
                        inet_ntoa(clientname.sin_addr),
                        ntohs(clientname.sin_port), client_sock);
            }

#if defined (AEE_TCP)
            if (!aee_connected && FD_ISSET(aee_listen_sock, &readfds)) {
                /* Connection request on original aee listen socket. */
                struct sockaddr_in clientname;
                size_t size = sizeof(clientname);

                if(g_aee_sock != -1)
                close(g_aee_sock);

                g_aee_sock = accept(aee_listen_sock,(struct sockaddr *) &clientname, &size);

                if (g_aee_sock < 0) {
                    LOG_ERROR("accept aee connection fail with error: %d\n", errno);
                    continue;
                }

                aee_connected = true;

                LOG_INFO("connect from aee %s, port %d.\n",
                        inet_ntoa(clientname.sin_addr),
                        ntohs(clientname.sin_port));

                write(g_aee_sock, "first message", 6);

            }
#elif defined (AEE_HECI)
            // TODO:
#endif
            /* handle message from host client */
            client = g_host_clients;
            while (client) {
                host_client_t * next_client = client->next;
                if (client->sock >= 0 && FD_ISSET(client->sock, &readfds)) {

                    numBytes = recv(client->sock, buffer, BUF_SIZE, 0);

                    /* the connection to a client is disconnected */
                    if (numBytes == 0
                            || (numBytes == -1 && errno == ECONNRESET)) {
                        LOG_WARNING("client sock %d disconnect!!!\n",
                                client->sock);
                        host_agent_client_clear_client(client);
                    } else if (numBytes > 0) {
                        LOG_INFO("recv %d data from client sock %d\n", numBytes,
                                client->sock);
                        handle_msg((char*) buffer, numBytes, &client->recv_ctx,
                                client);
                        // If reconnect, sock is copied, recv on the same sock again will block the thread, so break is better
                        break;
                    }
                }

                client = next_client;
            }

#if defined (AEE_TCP)
            /* handle message from aee */
            if (aee_connected && FD_ISSET(g_aee_sock, &readfds)) {
                numBytes = recv(g_aee_sock, buffer, BUF_SIZE, 0);
                if (numBytes == 0) {
                    aee_connected = false;
                } else if (numBytes == -1) {
                    if (errno == ECONNRESET)
                    aee_connected = false;
                    else
                    sock_errs ++;
                } else {
                    handle_msg((char*) buffer, numBytes, &aee_message_recv_ctx, NULL);
                }
            }
#elif defined (AEE_UART)
            if (FD_ISSET(g_aee_uart, &readfds)) {
                LOG_VERBOSE("ready to read data from AEE uart\n");
                numBytes = read(g_aee_uart, buffer, BUF_SIZE);
                if (numBytes == 0) {
                    sleep(3);
                } else if (numBytes == -1) {
                    LOG_ERROR("uart read error %d\n", errno);
                } else {
                    LOG_INFO("recv %d byte from AEE uart\n", numBytes);
                    handle_msg((char*) buffer, numBytes, &aee_message_recv_ctx, NULL);
                }
            }
#elif defined (AEE_AON)
            if (FD_ISSET(g_aee_aon, &readfds)) {
                LOG_VERBOSE("ready to read data from AEE aon pipe\n");
                numBytes = read(g_aee_aon, buffer, BUF_SIZE);
                if (numBytes == 0) {
                    sleep(3);
                } else if (numBytes == -1) {
                    LOG_ERROR("aon pipe read error %d\n", errno);
                } else {
                    LOG_INFO("recv %d byte from AEE aon pipe\n", numBytes);
                    handle_msg((char*) buffer, numBytes, &aee_message_recv_ctx, NULL);
                }
            }

#elif defined (AEE_HECI)
            // TODO:
#endif
        }
    }

    fail: cleanup();

    return 0;
}
