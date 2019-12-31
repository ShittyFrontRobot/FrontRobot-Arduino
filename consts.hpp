//
// Created by berberman on 2019/12/29.
//

#ifndef FRONTROBOT_CONSTS_HPP
#define FRONTROBOT_CONSTS_HPP

#include "ArduinoSTL.h"

constexpr static size_t motor_size = 6;
constexpr static uint8_t packet_head = 0xbc;

struct packet_info_t {
    const uint8_t type;
    const size_t size;

    constexpr size_t size_except_head() const { return size - 1; }
};


namespace nano {

    constexpr static packet_info_t
            motor_speed_packet_info{0xA1, 27},
            motor_state_packet_info{0xA3, 9},
            encoder_data_packet_info{0xA2, 27},
            encoder_reset_packet_info{0xA8, 27};

}

namespace stm {
    constexpr static packet_info_t
            encoder_data_packet_info{0x0D, 14},
            encoder_reset_packet_info{0x03, 4};
}


#endif //FRONTROBOT_CONSTS_HPP
