#include "keyboard.h"
#include "../kernel/interrupt.h"
#include "../lib/stdtypes.h"
#include "../lib/kernel/io.h"
#include "../lib/kernel/print.h"

#define KEYBOARD_BUF_PORT   0x60

/* ascii code of some control characters */
#define enter       '\n'
#define backspace   '\b'
#define tab         '\t'


/* scan code of control keys */
#define l_shift_make        0x2a
#define l_shift_break       0xaa
#define r_shift_make        0x36
#define r_shift_break       0xb6
#define caps_lock_make      0x3a

static uint8_t keymap[][2] = {
/* 0x00 */      { 0, 0 },
/* 0x01 */      { 0, 0 },
/* 0x02 */      { '1', '!' },
/* 0x03 */      { '2', '\"' },
/* 0x04 */      { '3', '#' },
/* 0x05 */      { '4', '$' },
/* 0x06 */      { '5', '%' },
/* 0x07 */      { '6', '&' },
/* 0x08 */      { '7', '\'' },
/* 0x09 */      { '8', '(' },
/* 0x0a */      { '9', ')' },
/* 0x0b */      { '0', '~' },
/* 0x0c */      { '-', '=' },
/* 0x0d */      { '0', '~' },
/* 0x0e */      { backspace, backspace },   /* backspace */
/* 0x0f */      { tab, tab },
/* 0x10 */      { 'q', 'Q' },
/* 0x11 */      { 'w', 'W' },
/* 0x12 */      { 'e', 'E' },
/* 0x13 */      { 'r', 'R' },
/* 0x14 */      { 't', 'T' },
/* 0x15 */      { 'y', 'Y' },
/* 0x16 */      { 'u', 'U' },
/* 0x17 */      { 'i', 'I' },
/* 0x18 */      { 'o', 'O' },
/* 0x19 */      { 'p', 'P' },
/* 0x1a */      { '[', '{' },
/* 0x1b */      { ']', '}' },
/* 0x1c */      { enter, enter },   /* enter */
/* 0x1d */      { 0, 0 },       /* left ctrl */
/* 0x1e */      { 'a', 'A' },
/* 0x1f */      { 's', 'S' },
/* 0x20 */      { 'd', 'D' },
/* 0x21 */      { 'f', 'F' },
/* 0x22 */      { 'g', 'G' },
/* 0x23 */      { 'h', 'H' },
/* 0x24 */      { 'j', 'J' },
/* 0x25 */      { 'k', 'K' },
/* 0x26 */      { 'l', 'L' },
/* 0x27 */      { ';', ':' },
/* 0x28 */      { ';', ':' },
/* 0x29 */      { ']', '}' },
/* 0x2a */      { 0, 0 },       /* left shift */
/* 0x2b */      { '\\', '_' },
/* 0x2c */      { 'z', 'Z' },
/* 0x2d */      { 'x', 'X' },
/* 0x2e */      { 'c', 'C' },
/* 0x2f */      { 'v', 'V' },
/* 0x30 */      { 'b', 'B' },
/* 0x31 */      { 'n', 'N' },
/* 0x32 */      { 'm', 'M' },
/* 0x33 */      { ',', '<' },
/* 0x34 */      { '.', '>' },
/* 0x35 */      { '/', '?' },
/* 0x36 */      { 0, 0 },       /* right shift */
/* 0x37 */      { 0, 0 },
/* 0x38 */      { 0, 0 },       /* left alt */
/* 0x39 */      { ' ', ' ' },   /* space */
/* 0x3a */      { 0, 0 },       /* caps lock */
};

static bool l_shift_down, r_shift_down, caps_lock;
static bool shift_reset;

static void keyboard_intr_handler(void) {
    uint8_t scancode = inb(KEYBOARD_BUF_PORT);
//    put_int(scancode);put_char('\n');
    switch (scancode) {
        // use left shift as caps lock
        case l_shift_make:
            if (shift_reset) {
                caps_lock = !caps_lock;
                shift_reset = false;
            }
            break;
        case l_shift_break:
            shift_reset = true;
            break;
        default:
            if (scancode < 0x40) {
                uint8_t ch = caps_lock ? keymap[scancode][1] : keymap[scancode][0];
                if (ch != 0) {
                    put_char(ch);
                }
            }
    }
    return;
}

void keyboard_init(void) {
    l_shift_down = false;
    r_shift_down = false;
    caps_lock = false;
    shift_reset = true;
    register_intr_handler(0x21, keyboard_intr_handler);
    put_str("keyboard init done\n");
}
