#include <Arduino.h>
#include "nano.hpp"
#include "motor.hpp"
#include "stm.hpp"


#include "parser.hpp"

motor_t<motor_core_t<2, 3>> chassis_lf;
motor_t<motor_core_t<4, 5>> chassis_lb;
motor_t<motor_core_t<6, 7>> chassis_rf;
motor_t<motor_core_t<8, 9>> chassis_rb;


void setup() {
    // PC
    Serial.begin(9600);
    // nano
    Serial1.begin(115200, SERIAL_8N1);
    // STM 1
    Serial2.begin(115200, SERIAL_8N1);
    // STM 2
    Serial3.begin(115200, SERIAL_8N1);
    printf_begin();
}

static int s[]{3, 2, 3, 4, 5, 6};

void run() {
    nano::receive_from(Serial1, [](float *a) {
        printf("speed: %f %f %f %f %f %f\n", a[0], a[1], a[2], a[3], a[4], a[5]);
    }, [](motor_state_t *a) {
        printf("state: %d %d %d %d %d %d\n", a[0], a[1], a[2], a[3], a[4], a[5]);
    }, []() {
        println("Reset");
    });
    nano::send_to(Serial1, s);
}


static uint8_t encoder_reset_packet[] = {188, 168, 168};
static uint8_t motor_speed_packet[] = {188, 161, 0, 0, 128, 63, 0, 0, 0, 64, 0, 0,
                                       64, 64, 0, 0, 128, 64, 0, 0, 160, 64,
                                       0, 0, 192, 64, 254};

static uint8_t motor_state_packet[] = {188, 163, 1, 1, 1, 1, 1, 1, 163};


void foo() {
    parse_engine_t<nano::parser_t> engine;
    engine(motor_speed_packet, motor_speed_packet + sizeof(motor_speed_packet), [](nano::parser_t::result_t &result) {
        printf("result: %d\n", static_cast<uint8_t>(result.type));
//        auto x = reinterpret_cast<float *>(result.speeds.data);
    });
}

void loop() {
    foo();
    delay(500);
}