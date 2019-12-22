#include <Arduino.h>
#include "nano.hpp"
#include "motor.hpp"

void setup() {
    Serial.begin(9600);
    Serial1.begin(115200);
    printf_begin();
}

void read_serial(HardwareSerial &serial) {
    auto byte = Serial1.read();
    if (byte != -1)
        printf("%d\n", byte);
}

void loop() {
    receive_from_nano(Serial1, [](float *) {}, [](motor_state_t *) {}, []() {});
    delay(5);
}
