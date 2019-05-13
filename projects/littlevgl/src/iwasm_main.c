#include "bh_platform.h"
#include "runtime_lib.h"
#include "native_interface.h"
#include "app-manager-export.h"

#include "bh_common.h"
#include "bh_queue.h"
#include "bh_thread.h"
#include "bh_memory.h"
#include "runtime_sensor.h"
#include "attr-container.h"
#include "module_wasm_app.h"
#include "wasm-export.h"

extern void * thread_timer_check(void *);
extern void init_sensor_framework();
extern int aee_host_msg_callback(void *msg, uint16_t msg_len);

#ifndef __ZEPHYR__
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

#define MAX 80
#define PORT 8888
#define SA struct sockaddr

int sockfd = -1;

// Function designed for chat between client and server.
void* func(void* arg)
{
    char buff[MAX];
    int n;
    int connfd, len;
    struct sockaddr_in servaddr, cli;

    while (1) {
        if (sockfd != -1)
            close(sockfd);
        // socket create and verification
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd == -1) {
            printf("socket creation failed...\n");
            return NULL;
        } else
            printf("Socket successfully created..\n");
        bzero(&servaddr, sizeof(servaddr));
        // assign IP, PORT
        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
        servaddr.sin_port = htons(PORT);

        // connect the client socket to server socket
        if (connect(sockfd, (SA*) &servaddr, sizeof(servaddr)) != 0) {
            printf("connection with the server failed...\n");
            sleep(10);
            continue;
        } else {
            printf("connected to the server..\n");

            host_send("hello", 6);
        }

        // infinite loop for chat
        for (;;) {
            bzero(buff, MAX);

            // read the message from client and copy it in buffer
            n = read(sockfd, buff, sizeof(buff));
            // print buffer which contains the client contents
            //fprintf(stderr, "recieved %d bytes from host: %s", n, buff);

            // socket disconnected
            if (n <= 0)
                break;

            aee_host_msg_callback(buff, n);
        }
    }

    // After chatting close the socket
    close(sockfd);
}

static bool host_init()
{
    return true;
}

int host_recv(void * ctx, char *buf, int buf_size)
{
    return 0;
}

int host_send(void * ctx, const char *buf, int size)
{
    if (sockfd == -1)
        return 0;

    return write(sockfd, buf, size);
}

void host_destroy()
{
    close(sockfd);
}
extern void display_SDL_init();
// Driver function

#else

#include <zephyr.h>
#include <uart.h>
#include <device.h>

#define HOST_LMT_COMM_UART_NAME "UART_1"
int uart_char_cnt = 0;

static void uart_irq_callback(struct device *dev)
{
    unsigned char ch;
    int size = 0;

    while (uart_poll_in(dev, &ch) == 0) {

        uart_char_cnt++;
        aee_host_msg_callback(&ch, 1);
    }

    /*app_manager_printf("Uart IRQ: total %d bytes received\n", size);*/
}

struct device *uart_dev = NULL;

static bool host_init()
{
    uart_dev = device_get_binding(HOST_LMT_COMM_UART_NAME);
    if (!uart_dev) {
        printf("UART: Device driver not found.\n");
        return;
    }
    uart_irq_rx_enable(uart_dev);
    uart_irq_callback_set(uart_dev, uart_irq_callback);
    return true;
}

int host_recv(void * ctx, char *buf, int buf_size)
{
    return 0;
}

int host_send(void * ctx, const char *buf, int size)
{
    for (int i = 0; i < size; i++)
    uart_poll_out(uart_dev, buf[i]);

    return size;
}

void host_destroy()
{

}

#endif

#define DEFAULT_THREAD_STACKSIZE (8 * 1024)

host_interface interface = { .init = host_init, .recv = host_recv, .send =
        host_send, .destroy = host_destroy };
timer_ctx_t timer_ctx;

static char global_heap_buf[1024 * 1024] = { 0 };

int iwasm_main()
{
    korp_thread tid, tm_tid;

    host_init();

    if (bh_memory_init_with_pool(global_heap_buf, sizeof(global_heap_buf))
            != 0) {
        printf("Init global heap failed.\n");
        return -1;
    }

    if (vm_thread_sys_init() != 0) {
        goto fail1;
    }

#ifndef __ZEPHYR__
    extern void display_SDL_init();
    display_SDL_init();
#else
    extern void display_init(void);
    display_init();
#endif

    // timer manager
    init_wasm_timer();

#ifndef __ZEPHYR__
    vm_thread_create(&tid, func, NULL, BH_APPLET_PRESERVED_STACK_SIZE);
#endif

    // TODO:
    app_manager_startup(&interface);

    fail1: bh_memory_destroy();
    return -1;
}
