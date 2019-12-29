//
// Created by berberman on 2019/12/22.
//

#ifndef FRONTROBOT_NANO_HPP
#define FRONTROBOT_NANO_HPP

#include "reader.hpp"
#include "consts.hpp"

/**
 * 来自 nano 的六电机速度、六电机状态、重置整体
 * @param serial
 * @param speed_callback
 * @param state_callback
 * @param reset_callback
 */
void receive_from_nano(HardwareSerial &serial, void (*speed_callback)(float *), void(*state_callback)(uint8_t *),
                       void(*reset_callback)()) {
    static uint8_t buffer[64];
    static size_t ptr = 0;

    // 如果是新包
    if (ptr == 0) {
        while (true) {
            auto byte = serial.read();
            // 前进到头为止
            if (byte == PACKET_HEAD) {
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
        if (buffer[0] != PACKET_NANO_TYPE_RESET && buffer[0] != PACKET_NANO_TYPE_STATE &&
            buffer[0] != PACKET_NANO_TYPE_SPEED) {
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
        case PACKET_NANO_TYPE_SPEED: {
            auto result = try_read<PACKET_NANO_SIZE_SPEED>(serial, buffer, ptr);
            ptr = result.ptr;
            if (result.success) {
                float speeds[MOTOR_SIZE];
                memcpy(speeds, buffer + 1, MOTOR_SIZE * sizeof(float));
                speed_callback(speeds);
            }
            break;
        }
        case PACKET_NANO_TYPE_STATE: {
            auto result = try_read<PACKET_NANO_SIZE_STATE>(serial, buffer, ptr);
            ptr = result.ptr;
            if (result.success) {
                uint8_t states[MOTOR_SIZE];
                memcpy(states, buffer + 1, MOTOR_SIZE * sizeof(uint8_t));
                state_callback(states);
            }
            break;
        }

        case PACKET_NANO_TYPE_RESET: {
            auto result = try_read<PACKET_NANO_SIZE_RESET>(serial, buffer, ptr);
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
void send_to_nano(HardwareSerial &serial, float *encoder_values) {
    uint8_t payload[PACKET_NANO_SIZE_ENCODER + 1];
    payload[0] = PACKET_HEAD;
    payload[1] = PACKET_NANO_TYPE_ENCODER;
    payload[PACKET_NANO_SIZE_ENCODER] = 0;
    memcpy(payload + 2, encoder_values, MOTOR_SIZE * sizeof(float));
    for (auto i = 1; i < PACKET_NANO_SIZE_ENCODER; ++i) {
        payload[PACKET_NANO_SIZE_ENCODER] ^= payload[i];
    }
    serial.write(payload, PACKET_NANO_SIZE_ENCODER + 1);
}

#endif //FRONTROBOT_NANO_HPP
