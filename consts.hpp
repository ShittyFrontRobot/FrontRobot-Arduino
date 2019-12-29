//
// Created by berberman on 2019/12/29.
//

#ifndef FRONTROBOT_CONSTS_HPP
#define FRONTROBOT_CONSTS_HPP

#define MOTOR_SIZE 6
#define PACKET_HEAD 0xbc

//region NANO

#define PACKET_NANO_TYPE_SPEED 0xA1
// Motor size * sizeOf(float) + 3(0xbc + type + check) - (0xbc)
#define PACKET_NANO_SIZE_SPEED 26
#define PACKET_NANO_TYPE_STATE 0xA3
// Motor size * sizeOf(byte) +  3(0xbc + type + check) - (0xbc)
#define PACKET_NANO_SIZE_STATE 8
#define PACKET_NANO_TYPE_RESET 0xA8
// Motor size * 0            +  3(0xbc + type + check) - (0xbc)
#define PACKET_NANO_SIZE_RESET 2
#define PACKET_NANO_TYPE_ENCODER 0xA2
// Motor size * sizeOf(float) + 3(0xbc + type + check) - (0xbc)
#define PACKET_NANO_SIZE_ENCODER 26

//endregion

//region STM

#define PACKET_STM_TYPE_ENCODER 13
// Motor size /2 * sizeOf(float) + 3(0xbc + type + check) - (0xbc)
#define PACKET_STM_SIZE_ENCODER 14

#define PACKET_STM_TYPE_RESET 3
// Motor size * 0                + 3(0xbc + type + check) - (0xbc)
#define PACKET_STM_SIZE_RESET 4

//endregion

#endif //FRONTROBOT_CONSTS_HPP
