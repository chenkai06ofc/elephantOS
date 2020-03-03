#include "keyboard.h"
#include "../kernel/interrupt.h"
#include "../lib/kernel/io.h"
#include "../lib/kernel/print.h"

#define KEYBOARD_BUF_PORT   0x60

static void keyboard_intr_handler(void) {
    put_char('k');
    inb(KEYBOARD_BUF_PORT);
    return;
}

void keyboard_init(void) {
    register_intr_handler(0x21, keyboard_intr_handler);
    put_str("keyboard init done\n");
}
