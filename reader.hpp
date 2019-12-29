//
// Created by berberman on 12/28/2019.
//

#ifndef FRONTROBOT_READER_HPP
#define FRONTROBOT_READER_HPP

#include "logger.hpp"

/**
 * 串口读取结果
 */
struct read_result_t {
    bool success;
    size_t ptr;
};

/**
 * 读串口
 * @tparam size 读取长度
 * @param serial
 * @param buffer
 * @param ptr
 * @return 新指针位置
 */
template<size_t size>
read_result_t try_read(HardwareSerial &serial, uint8_t *buffer, size_t ptr) {
    auto read_size = size - ptr;
    println_debug("=======READING START=======");
    // 尝试读对应长度
    printf_debug("I need to read %d byte(s) this time.\n", read_size);
    auto actual_size = serial.readBytes(buffer + 1, read_size);
    printf_debug("And I got %d.\n", actual_size);

    ptr += actual_size;

    println_debug("Buffer: ");
    print_buf_debug(buffer, 40);

    // 没读完等下次
    if (ptr < size) {
        printf_debug("I've got %d byte(s) so far, but it should be %d.\n", ptr, size);
        println_debug("Well, I need more.");
        println_debug("=======READING END=========");
        return read_result_t{
                false, ptr
        };
    }
    printf_debug("I have %d byte(s) of current packet now.\n", ptr);
    println_debug("Good, finish reading.");
    println_debug("Now begin to check:");
    print_buf_debug(buffer, size);
    uint8_t check = 0;
    // 去掉最后一位（校验位）异或
    for (auto i = static_cast<int>(ptr) - 2; i >= 0; --i) {
        check ^= buffer[i];
    }
    printf_debug("Expect: %d, Actual: %d.\n", buffer[ptr - 1], check);

    // 与最后一位对比
    if (check != buffer[ptr - 1]) {
        println_debug("Checking failed, ready to receive new packets.");
        println_debug("=======READING END=========");
        return read_result_t{
                false, 0
        };
    }
    println_debug("Checking finished.");
    println_debug("=======READING END=========");
    println_debug("It is time to handle callbacks.");
    return read_result_t{
            true, 0
    };
}

#endif //FRONTROBOT_READER_HPP
