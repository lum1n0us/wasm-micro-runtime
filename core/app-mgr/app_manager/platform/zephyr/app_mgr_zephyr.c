#include "app-manager.h"
#include "bh_platform.h"
#include "bh_memory.h"
#include <autoconf.h>
#include <zephyr.h>
#include <kernel.h>
#if 0
#include <sigverify.h>
#endif
typedef struct k_timer_watchdog {
    struct k_timer timer;
    watchdog_timer *wd_timer;
} k_timer_watchdog;

void*
app_manager_timer_create(void (*timer_callback)(void*),
        watchdog_timer *wd_timer)
{
    struct k_timer_watchdog *timer = bh_malloc(sizeof(struct k_timer_watchdog));

    if (timer) {
        k_timer_init(&timer->timer, (void (*)(struct k_timer*)) timer_callback,
        NULL);
        timer->wd_timer = wd_timer;
    }

    return timer;
}

void app_manager_timer_destroy(void *timer)
{
    bh_free(timer);
}

void app_manager_timer_start(void *timer, int timeout)
{
    k_timer_start(timer, timeout, 0);
}

void app_manager_timer_stop(void *timer)
{
    k_timer_stop(timer);
}

watchdog_timer *
app_manager_get_wd_timer_from_timer_handle(void *timer)
{
    return ((k_timer_watchdog*) timer)->wd_timer;
}
#if 0
int app_manager_signature_verify(const uint8_t *file, unsigned int file_len,
        const uint8_t *signature, unsigned int sig_size)
{
    return signature_verify(file, file_len, signature, sig_size);
}
#endif
