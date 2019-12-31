//
// Created by berberman on 2019/12/22.
//

#ifndef FRONTROBOT_NANO_HPP
#define FRONTROBOT_NANO_HPP

#include "reader.hpp"
#include "consts.hpp"
#include "motor.hpp"

namespace nano {

/**
 * 来自 nano 的六电机速度、六电机状态、重置整体
 * @param serial
 * @param speed_callback
 * @param state_callback
 * @param reset_callback
 */
    void receive_from(HardwareSerial &serial, void (*speed_callback)(float *), void(*state_callback)(motor_state_t *),
                      void(*reset_callback)()) {
        static uint8_t buffer[64];
        static size_t ptr = 0;

        // 如果是新包
        if (ptr == 0) {
            while (true) {
                auto byte = serial.read();
                // 前进到头为止
                if (byte == packet_head) {
                    break;
                } else if (byte == -1) {
                    return;
                }
            }
            // 拿到类型
            buffer[0] = serial.read();
            ptr += 1;
            printf_debug("A new packet has come, type: %d.\n", buffer[0]);

            // 错了就扔掉
            if (buffer[0] != encoder_reset_packet_info.type &&
                buffer[0] != motor_state_packet_info.type &&
                buffer[0] != motor_speed_packet_info.type) {
                ptr = 0;
                println("Error! Unknown packet type!");
                return;
            }


        } else {
            // 如果没读出来等下次
            if (serial.peek() == -1)
                return;

            println_debug("Current packet is not finished, resume with it.");
        }


        switch (buffer[0]) {
            case motor_speed_packet_info.type: {
                auto result = try_read<motor_speed_packet_info.size_except_head()>(serial, buffer, ptr);
                ptr = result.ptr;
                if (result.success) {
                    float speeds[motor_size];
                    memcpy(speeds, buffer + 1, motor_size * sizeof(float));
                    speed_callback(speeds);
                }
                break;
            }
            case motor_state_packet_info.type: {
                auto result = try_read<motor_state_packet_info.size_except_head()>(serial, buffer, ptr);
                ptr = result.ptr;
                if (result.success) {
                    uint8_t states[motor_size];
                    memcpy(states, buffer + 1, motor_size * sizeof(uint8_t));
                    state_callback(reinterpret_cast<motor_state_t *>(states));
                }
                break;
            }

            case encoder_reset_packet_info.type: {
                auto result = try_read<encoder_reset_packet_info.size_except_head()>(serial, buffer, ptr);
                ptr = result.ptr;
                if (result.success)
                    reset_callback();
                break;
            }
        }
    }

/**
 * 发给 nano 的六编码器脉冲
 * @param serial
 * @param encoder_values
 */
    void send_to(HardwareSerial &serial, int *encoder_values) {
        constexpr static uint8_t size = encoder_data_packet_info.size_except_head();
        uint8_t payload[size + 1];
        payload[0] = packet_head;
        payload[1] = encoder_data_packet_info.type;
        payload[size] = 0;
        memcpy(payload + 2, encoder_values, motor_size * sizeof(int));
        for (auto i = 1; i < size; ++i) {
            payload[size] ^= payload[i];
        }
        serial.write(payload, size + 1);
    }
}

#endif //FRONTROBOT_NANO_HPP
