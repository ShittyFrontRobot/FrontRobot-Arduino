//
// Created by berberman on 12/28/2019.
//

#ifndef FRONTROBOT_STM_HPP
#define FRONTROBOT_STM_HPP

#include "consts.hpp"

/**
 * 来自 STM 的三编码器脉冲
 * @param serial
 * @param encoder_data_callback 大小为 3
 */
void receive_from_stm(HardwareSerial &serial, void (*encoder_data_callback)(int *)) {
    static uint8_t buffer[64];
    static size_t ptr = 0;

    if (ptr == 0) {
        while (true) {
            auto byte = serial.read();
            if (byte == PACKET_HEAD) {
                break;
            } else if (byte == -1) {
                return;
            }
        }

        buffer[0] = serial.read();
        ptr += 1;

        if (buffer[0] != PACKET_STM_TYPE_ENCODER) {
            ptr = 0;
            return;
        }

    } else {
        if (serial.peek() == -1)
            return;
    }

    auto result = try_read<PACKET_STM_SIZE_ENCODER>(serial, buffer, ptr);
    ptr = result.ptr;
    if (result.success) {
        int values[MOTOR_SIZE];
        memcpy(values, buffer + 1, MOTOR_SIZE * sizeof(int));
        encoder_data_callback(values);
    }

}

/**
 * 发给 STM 的重置编码器
 * @param serial
 */
void send_to_stm(HardwareSerial &serial) {
    uint8_t payload[PACKET_STM_SIZE_RESET + 1];
    payload[0] = PACKET_HEAD;
    payload[1] = PACKET_STM_TYPE_RESET;
    payload[2] = 0x5A;
    payload[3] = 0x69;
    payload[PACKET_STM_SIZE_RESET] = 0x30;
    serial.write(payload, PACKET_STM_SIZE_RESET + 1);
}

#endif //FRONTROBOT_STM_HPP
