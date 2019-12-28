#include <Arduino.h>
#include "nano.hpp"
#include "motor.hpp"

void setup() {
    Serial.begin(9600);
    Serial1.begin(115200, SERIAL_8N1);
    printf_begin();
}

void read_serial(HardwareSerial &serial) {
    auto byte = Serial1.read();
    if (byte != -1)
        printf("%d\n", byte);
}

static float s[]{3, 2, 3, 4, 5, 6};

void loop() {
    receive_from_nano(Serial1, [](float *a) {
        printf("speed: %f %f %f %f %f %f\n", a[0], a[1], a[2], a[3], a[4], a[5]);
    }, [](motor_state_t *a) {
        printf("state: %d %d %d %d %d %d\n", a[0], a[1], a[2], a[3], a[4], a[5]);
    }, []() {
        println("Reset");
    });
    send_to_nano(Serial1, s);
    delay(50);
}
