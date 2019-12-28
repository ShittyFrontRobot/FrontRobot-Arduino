//
// Created by berberman on 2019/12/22.
//

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#ifndef FRONTROBOT_LOGGER_HPP
#define FRONTROBOT_LOGGER_HPP

#include "Arduino.h"

#ifdef PRINT
#define printf_debug(...)  printf(__VA_ARGS__)
#else
#define printf_debug(...) foo()
#endif


void foo() {
    // Nothing to do
}

int serial_print(char c, struct __file *) {
    Serial.write(c);
    return c;
}

void printf_begin() {
    fdevopen(&serial_print, nullptr);
}

void println(const char *string) {
    printf("%s\n", string);
}
void print(const char *string) {
    printf("%s", string);
}

void println_debug(const char *string) {
    printf_debug("%s\n", string);
}

void print_debug(const char *string) {
    printf_debug("%s", string);
}

void print_buf_debug(uint8_t *buf, size_t size) {
    println_debug("==========BUF==========");
    for (auto i = 0; i < size; i++) {
        printf_debug("%d", buf[i]);
        if (i != size - 1)
            print_debug(",");
    }

    println_debug("\n==========BUF==========");
}

void print_buf(uint8_t *buf, size_t size) {
    println("==========BUF==========");
    for (auto i = 0; i < size; i++) {
        printf("%d", buf[i]);
        if (i != size - 1)
            print(",");
    }

    println("\n==========BUF==========");
}


#endif //FRONTROBOT_LOGGER_HPP

#pragma clang diagnostic pop