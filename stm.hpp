//
// Created by berberman on 12/28/2019.
//

#ifndef FRONTROBOT_STM_HPP
#define FRONTROBOT_STM_HPP

#include "consts.hpp"

namespace stm {
/**
 * 来自 STM 的三编码器脉冲
 * @param serial
 * @param encoder_data_callback 大小为 3
 */
    void receive_from(HardwareSerial &serial, void (*encoder_data_callback)(int *)) {
        static uint8_t buffer[64];
        static size_t ptr = 0;

        if (ptr == 0) {
            while (true) {
                auto byte = serial.read();
                if (byte == packet_head) {
                    break;
                } else if (byte == -1) {
                    return;
                }
            }

            buffer[0] = serial.read();
            ptr += 1;

            if (buffer[0] != encoder_data_packet_info.type) {
                ptr = 0;
                return;
            }

        } else {
            if (serial.peek() == -1)
                return;
        }

        auto result = try_read<encoder_data_packet_info.size_except_head()>(serial, buffer, ptr);
        ptr = result.ptr;
        if (result.success) {
            int values[motor_size];
            memcpy(values, buffer + 1, motor_size * sizeof(int));
            encoder_data_callback(values);
        }

    }

/**
 * 发给 STM 的重置编码器
 * @param serial
 */
    void send_to_stm(HardwareSerial &serial) {
        constexpr static uint8_t size = encoder_reset_packet_info.size_except_head();
        uint8_t payload[size + 1];
        payload[0] = packet_head;
        payload[1] = encoder_reset_packet_info.type;
        payload[2] = 0x5A;
        payload[3] = 0x69;
        payload[size] = 0x30;
        serial.write(payload, size + 1);
    }
}
#endif //FRONTROBOT_STM_HPP
