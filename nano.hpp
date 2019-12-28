//
// Created by berberman on 2019/12/22.
//

#ifndef FRONTROBOT_NANO_HPP
#define FRONTROBOT_NANO_HPP

#include "reader.hpp"

#define MOTOR_SIZE 6

#define PACKET_TYPE_SPEED 0xA1
// Motor size * sizeOf(float) + 3(0xbc + type + check) - (0xbc)
#define PACKET_SPEED_SIZE 26
#define TYPE_STATE 0xA3
// Motor size * sizeOf(byte) + 3(0xbc + type + check) - (0xbc)
#define PACKET_STATE_SIZE 8
#define PACKET_TYPE_RESET 0xA8
// Motor size * 0            + 3(0xbc + type + check) - (0xbc)
#define PACKET_RESET_SIZE 2
#define PACKET_TYPE_ENCODER 0xA2
// Motor size * sizeOf(float) + 3(0xbc + type + check) - (0xbc)
#define PACKET_ENCODER_SIZE 26


void receive_from_nano(HardwareSerial &serial, void (*speed_callback)(float *), void(*state_callback)(uint8_t *),
                       void(*reset_callback)()) {
    static uint8_t buffer[64];
    static size_t ptr = 0;

    // 如果是新包
    if (ptr == 0) {
        while (true) {
            auto byte = serial.read();
            // 前进到头为止
            if (byte == 0xbc) {
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
        if (buffer[0] != PACKET_TYPE_RESET && buffer[0] != TYPE_STATE && buffer[0] != PACKET_TYPE_SPEED) {
            ptr = 0;
            println("Error! the first byte can't be 0xbc or 0xff or sth unknown!");
            return;
        }


    } else {
        // 如果没读出来等下次
        if (serial.peek() == -1)
            return;

        println_debug("Current packet is not finished, resume with it.");
    }


    switch (buffer[0]) {
        case PACKET_TYPE_SPEED: {
            auto result = try_read<PACKET_SPEED_SIZE>(serial, buffer, ptr);
            ptr = result.ptr;
            if (result.success) {
                float speeds[MOTOR_SIZE];
                memcpy(speeds, buffer + 1, MOTOR_SIZE * sizeof(float));
                speed_callback(speeds);
            }
            break;
        }
        case TYPE_STATE: {
            auto result = try_read<PACKET_STATE_SIZE>(serial, buffer, ptr);
            ptr = result.ptr;
            if (result.success) {
                uint8_t states[MOTOR_SIZE];
                memcpy(states, buffer + 1, MOTOR_SIZE * sizeof(uint8_t));
                state_callback(states);
            }
            break;
        }

        case PACKET_TYPE_RESET: {
            auto result = try_read<PACKET_RESET_SIZE>(serial, buffer, ptr);
            ptr = result.ptr;
            if (result.success)
                reset_callback();
            break;
        }
    }
}

void send_to_nano(HardwareSerial &serial, float *encoder_values) {
    uint8_t payload[PACKET_ENCODER_SIZE + 1];
    payload[0] = 0xbc;
    payload[1] = PACKET_TYPE_ENCODER;
    payload[PACKET_ENCODER_SIZE] = 0;
    memcpy(payload + 2, encoder_values, MOTOR_SIZE * sizeof(float));
    for (auto i = 1; i < PACKET_ENCODER_SIZE; ++i) {
        payload[PACKET_ENCODER_SIZE] ^= payload[i];
    }
    serial.write(payload, PACKET_ENCODER_SIZE + 1);
}

#endif //FRONTROBOT_NANO_HPP
