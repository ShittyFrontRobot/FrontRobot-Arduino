//
// Created by berberman on 12/28/2019.
//

#ifndef FRONTROBOT_STM_HPP
#define FRONTROBOT_STM_HPP

void receive_from_stm(HardwareSerial &serial, void (*encoder_data_callback)(float *)) {
    static uint8_t buffer[64];
    static size_t ptr = 0;

    if (ptr == 0) {
        while (true) {
            auto byte = serial.read();
            if (byte == 0xbc) {
                break;
            } else if (byte == -1) {
                return;
            }
        }
    }

#endif //FRONTROBOT_STM_HPP
