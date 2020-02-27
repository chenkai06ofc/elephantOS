#include "timer.h"
#include "../kernel/interrupt.h"
#include "../lib/stdint.h"
#include "../lib/kernel/io.h"
#include "../lib/kernel/print.h"
#include "../thread/thread.h"

#define IRQ0_FREQUENCY      10
#define INPUT_FREQUENCY     1193180
#define COUNTER0_VALUE      60000
#define COUNTER0_PORT       0x40
#define COUNTER0_NO         0
#define COUNTER_MODE        2
#define READ_WRITE_LATCH    3
#define PIT_CONTROL_PORT    0x43

static void frequency_set(uint8_t counter_port,
                            uint8_t counter_no,
                            uint8_t rwl,
                            uint8_t counter_mode,
                            uint16_t counter_value) {
    outb(PIT_CONTROL_PORT, (uint8_t)(counter_no << 6 | rwl << 4 | counter_mode << 1));
    outb(counter_port, (uint8_t)counter_value);
    outb(counter_port, (uint8_t)counter_value >> 8);
}

static void timer_intr_handler(void) {
    struct task_struct* current = current_thread();
    current->ticks--;
    current->elapsed_ticks++;
    if (current->ticks == 0) {
        schedule();
    }
}

void timer_init(void) {
    frequency_set(COUNTER0_PORT, COUNTER0_NO, READ_WRITE_LATCH, COUNTER_MODE, COUNTER0_VALUE);
    register_intr_handler(0x20, timer_intr_handler);
    put_str("timer_init done\n");
}

