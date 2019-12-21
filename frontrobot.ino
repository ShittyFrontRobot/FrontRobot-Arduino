#include <Arduino.h>
#include "motor.hpp"


#define MOTOR_SIZE 6

#define TYPE_SPEED 0xA1
// Motor size * 4 + 3 -1
#define SPEED_SIZE 26
#define TYPE_STATE 0xA3
// Motor size * 1 + 3 -1
#define STATE_SIZE 8
#define TYPE_RESET 0xA8
// Motor size * 0 + 3 -1
#define RESET_SIZE 2

void speed_callback(const float *);

void state_callback(const motor_state_t *);

void reset_callback();

void print_buf(uint8_t *buf, size_t size);


struct read_result_t {
    bool success;
    size_t ptr;
};

template<size_t size>
read_result_t try_read(HardwareSerial &serial, uint8_t *buffer, size_t ptr) {
    Serial.println("=======READING START=======");
    // 尝试读对应长度
    Serial.print("I need read ");
    Serial.print(size - ptr);
    Serial.println(" byte(s) this time");
    ptr += serial.readBytes(buffer + 1, size - ptr);
    // 没读完下次再说
    if (ptr < size) {
        Serial.print("I've got ");
        Serial.print(ptr);
        Serial.print(" byte(s) so far, but it should be ");
        Serial.println(size);
        Serial.println("Well, I need more");
        Serial.println("=======READING END=========");
        return read_result_t{
                false, ptr
        };
    }
    Serial.print("I have ");
    Serial.print(ptr);
    Serial.println(" byte(s) now");
    Serial.println("Good, finish reading");
    Serial.println("Now begin to check:");
    print_buf(buffer, size);
    uint8_t check = 0;
    // 去掉最后一位（校验位）异或
    for (auto i = static_cast<int>(ptr) - 2; i >= 0; --i) {
        check ^= buffer[i];
    }
    Serial.print("Expect: ");
    Serial.println(buffer[ptr - 1]);

    Serial.print("Actual: ");
    Serial.println(check);
    // 与最后一位对比
    if (check != buffer[ptr - 1]) {
        Serial.println("Checking failed, ready to receive new packets");
        Serial.println("=======READING END=========");
        return read_result_t{
                false, 0
        };
    }
    Serial.println("Checking finished");
    Serial.println("=======READING END=========");
    Serial.println("It is time to handle callbacks");
    return read_result_t{
            true, 0
    };
}

void receive_from_nano(HardwareSerial &serial) {
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
        // 类型
        buffer[0] = serial.read();
        Serial.println("A new packet has come");
        Serial.print("Type: ");
        Serial.println(buffer[0]);
        // TODO: ???
        if (buffer[0] == 0xbc) {
            ptr = 0;
            Serial.println("Error! the first byte can't be 0xff!");
            return;
        }
    } else {
        Serial.println("Last packet is not finished, resume with it");
    }


    switch (buffer[0]) {
        case TYPE_SPEED: {
            auto result = try_read<SPEED_SIZE>(serial, buffer, ptr);
            ptr = result.ptr;
            if (result.success) {
                float speeds[MOTOR_SIZE];
                memcpy(speeds, buffer + 1, MOTOR_SIZE * sizeof(float));
                speed_callback(speeds);
            }
            break;
        }
        case TYPE_STATE: {
            auto result = try_read<STATE_SIZE>(serial, buffer, ptr);
            ptr = result.ptr;
            if (result.success) {
                motor_state_t states[MOTOR_SIZE];
                memcpy(states, buffer + 1, MOTOR_SIZE * sizeof(uint8_t));
                state_callback(states);
            }
            break;
        }

        case TYPE_RESET: {
            auto result = try_read<RESET_SIZE>(serial, buffer, ptr);
            ptr = result.ptr;
            if (result.success)
                reset_callback();
            break;
        }
    }
}

void print_buf(uint8_t *buf, size_t size) {
    Serial.println("==========BUF==========");
    for (auto i = 0; i < size; i++) {
        Serial.print(buf[i]);
        Serial.print(",");
    }
    Serial.println("\n==========BUF==========");
}

void speed_callback(const float *speeds) {
    Serial.println("Motor speeds: ");
    for (auto i = MOTOR_SIZE - 1; i >= 0; --i) {
        Serial.println(speeds[i]);
    }
}

void state_callback(const uint8_t *states) {
    Serial.println("Motor states: ");
    for (auto i = MOTOR_SIZE - 1; i >= 0; --i) {
        Serial.println(states[i]);
    }
}

void reset_callback() {
    Serial.println("reset");
}

void setup() {
    Serial.begin(9600);
    Serial1.begin(115200);
}

void loop() {
    receive_from_nano(Serial1);
//    auto byte=Serial1.read();
//    if(byte!=-1)
//    Serial.println(byte);
    delay(10);
}
