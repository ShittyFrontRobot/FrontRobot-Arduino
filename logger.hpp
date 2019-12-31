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
#define printf_debug(...) __WDNMD__()
#endif

/**
 * 孤儿
 */
void __WDNMD__() {
    // Nothing to do
}

/**
 * 不需要手动调用
 * @param c
 * @return
 */
int serial_print(char c, struct __file *) {
    Serial.write(c);
    return c;
}
/**
 * 不需要手动调用
 */
void printf_begin() {
    fdevopen(&serial_print, nullptr);
}

/**
 * 向 USB 串口打印
 * @param string
 */
void println(const char *string) {
    printf("%s\n", string);
}

/**
 * 向 USB 串口打印
 * @param string
 */
void print(const char *string) {
    printf("%s", string);
}

/**
 * 向 USB 串口字节数组
 * @param buf
 * @param size
 */
void print_buf(uint8_t *buf, size_t size) {
    println("==========BUF==========");
    for (auto i = 0; i < size; i++) {
        printf("%d", buf[i]);
        if (i != size - 1)
            print(",");
    }

    println("\n==========BUF==========");
}

//region DEBUG

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

//endregion



#endif //FRONTROBOT_LOGGER_HPP

#pragma clang diagnostic pop