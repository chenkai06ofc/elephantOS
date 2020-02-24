#include "debug.h"
#include "interrupt.h"
#include "../lib/kernel/print.h"

void panic_spin(char* filename, int line, const char* func, char* condition) {
    intr_disable();
    put_str("---------- error !!! ----------\n");
    put_str("filename: ");put_str(filename);put_str("\n");
    put_str("line: 0x");put_int(line);put_str("\n");
    put_str("function: ");put_str((char*)func);put_str("\n");
    put_str("condition: ");put_str(condition);put_str("\n");
    while(1);
}
