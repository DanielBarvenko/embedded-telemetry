#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/__assert.h>
#include "alarm.h"
#include "telemetry.h"

#define CAPACITY 4
#define SAMPLE_COUNT 12
#define SAMPLE_PERIOD_MS 100
#define CONSUMER_DELAY_MS 250

/*
Queue copies the snapshot
Neither task shares mutable sample storage
*/
struct message {
    sample_t sample;
    int alarm;
    int end;
};

K_MSGQ_DEFINE(samples, sizeof(struct message), CAPACITY, 4);
K_SEM_DEFINE(start_sampling, 0, 1);
K_SEM_DEFINE(start_output, 0, 1);

/* 
Written by only one worker each
Main reads after joining both workers
*/
static unsigned accepted;
static unsigned dropped;
static unsigned transmitted;
static int final_alarm;

static void sample_task(void *a, void *b, void *c) {
    (void)a; 
    (void)b; 
    (void)c;

    const int16_t temperatures[] = {2500, 2950, 3100, 2900, 2750, -125, 3200, 2900, 2800, 3050, 2850, 2700};

    int alarm = 0;
    k_sem_take(&start_sampling, K_FOREVER);

    for (uint16_t seq = 0; seq < SAMPLE_COUNT; seq++) {
        alarm = alarm_update(alarm, temperatures[seq]);

        struct message item = {
            .sample = {.sequence = seq, .temperature = temperatures[seq]},
            .alarm = alarm, .end = 0
        };

        /* 
        Never block acquisition on queue space
        Preserve older samples 
        */
        if (k_msgq_put(&samples, &item, K_NO_WAIT) == 0) {
            accepted++;
        } else {
            dropped++;
        }
        /* 
        Controlled startup backlog
        First six samples guarantee two drops
        */
        if (seq == 5) {
            k_sem_give(&start_output);
        }
        k_msleep(SAMPLE_PERIOD_MS);
    }

    final_alarm = alarm;

    /* 
    End marker is control traffic
    It must be delivered, so it may wait
    */

    const struct message end = {.end = 1};
    int rc = k_msgq_put(&samples, &end, K_FOREVER);

    __ASSERT(rc == 0, "end marker enqueue failed");
}

static void output_task(void *a, void *b, void *c) {
    (void)a; 
    (void)b; 
    (void)c;

    static const char digits[] = "0123456789abcdef";
    k_sem_take(&start_output, K_FOREVER);

    while (1) {
        struct message item;

        int rc = k_msgq_get(&samples, &item, K_FOREVER);
        __ASSERT(rc == 0, "queue receive failed");

        if (item.end) {
            break;
        }

        uint8_t frame[FRAME_SIZE];
        char hex[FRAME_SIZE * 2 + 1];
        frame_encode(frame, item.sample);

        for (size_t i = 0; i < FRAME_SIZE; i++) {
            hex[2 * i] = digits[frame[i] >> 4];
            hex[2 * i + 1] = digits[frame[i] & 15];
        }

        hex[FRAME_SIZE * 2] = '\0';
        
        printk("FRAME seq=%u temp=%d alarm=%d hex=%s\n", (unsigned)item.sample.sequence, 
                (int)item.sample.temperature, item.alarm, hex);
        
        transmitted++;
        k_msleep(CONSUMER_DELAY_MS);
    }
}

K_THREAD_DEFINE(sampler, 1536, sample_task, NULL, NULL, NULL, 4, 0, 0);
K_THREAD_DEFINE(output, 1536, output_task, NULL, NULL, NULL, 5, 0, 0);

int main(void) {
    printk("TELEMETRY RTOS demo: capacity=%d sample_ms=%d consumer_ms=%d\n", CAPACITY, SAMPLE_PERIOD_MS, CONSUMER_DELAY_MS);

    k_sem_give(&start_sampling);

    int rc = k_thread_join(sampler, K_FOREVER);
    __ASSERT(rc == 0, "sampler join failed");

    rc = k_thread_join(output, K_FOREVER);
    __ASSERT(rc == 0, "output join failed");

    int ok = accepted + dropped == SAMPLE_COUNT && transmitted == accepted && dropped >= 2 && final_alarm == 0;

    printk("SUMMARY generated=%d accepted=%u dropped=%u transmitted=%u alarm=%d status=%s\n",
           SAMPLE_COUNT, accepted, dropped, transmitted, final_alarm, ok ? "PASS" : "FAIL");

    /* 
    Returning ends main
    The kernel idle thread remains alive in QEMU
    */
    return ok ? 0 : 1;
}
