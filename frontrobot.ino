#include <Arduino.h>
#include "motor.hpp"
#include "logger.hpp"

#define MOTOR_SIZE 6

#define TYPE_SPEED 0xA1
// Motor size * sizeOf(float) + 3(0xbc + type + check) - (0xbc)
#define SPEED_SIZE 26
#define TYPE_STATE 0xA3
// Motor size * sizeOf(byte) + 3(0xbc + type + check) - (0xbc)
#define STATE_SIZE 8
#define TYPE_RESET 0xA8
// Motor size * 0            + 3(0xbc + type + check) - (0xbc)
#define RESET_SIZE 2

void speed_callback(const float *);

void state_callback(const motor_state_t *);

void reset_callback();


struct read_result_t {
    bool success;
    size_t ptr;
};

template<size_t size>
read_result_t try_read(HardwareSerial &serial, uint8_t *buffer, size_t ptr) {
    auto read_size = size - ptr;
    println_debug("=======READING START=======");
    // 尝试读对应长度
    printf_debug("I need to read %d byte(s) this time.\n", read_size);
    auto actual_size = serial.readBytes(buffer + 1, read_size);
    printf_debug("And I got %d.\n", actual_size);

    ptr += actual_size;

    println_debug("Buffer: ");
    print_buf_debug(buffer, 40);

    // 没读完等下次
    if (ptr < size) {
        printf_debug("I've got %d byte(s) so far, but it should be %d.\n", ptr, size);
        println_debug("Well, I need more.");
        println_debug("=======READING END=========");
        return read_result_t{
                false, ptr
        };
    }
    printf_debug("I have %d byte(s) of current packet now.\n", ptr);
    println_debug("Good, finish reading.");
    println_debug("Now begin to check:");
    print_buf_debug(buffer, size);
    uint8_t check = 0;
    // 去掉最后一位（校验位）异或
    for (auto i = static_cast<int>(ptr) - 2; i >= 0; --i) {
        check ^= buffer[i];
    }
    printf_debug("Expect: %d, Actual: %d.\n", buffer[ptr - 1], check);

    // 与最后一位对比
    if (check != buffer[ptr - 1]) {
        println_debug("Checking failed, ready to receive new packets.");
        println_debug("=======READING END=========");
        return read_result_t{
                false, 0
        };
    }
    println_debug("Checking finished.");
    println_debug("=======READING END=========");
    println_debug("It is time to handle callbacks.");
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
        // 拿到类型
        buffer[0] = serial.read();
        ptr += 1;
        printf("A new packet has come, type: %d.\n", buffer[0]);

        // 错了就扔掉
        if (buffer[0] != TYPE_RESET && buffer[0] != TYPE_STATE && buffer[0] != TYPE_SPEED) {
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

int i = 0;
int last = i;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"

void speed_callback(const float *speeds) {

//    println("Motor speeds: ");
//    for (auto i = MOTOR_SIZE - 1; i >= 0; --i) {
//        println(speeds[i]);
//    }
    i++;
}

void state_callback(const uint8_t *states) {
//    println("Motor states: ");
//    for (auto i = MOTOR_SIZE - 1; i >= 0; --i) {
//        println(states[i]);
//    }
    ++i;
}

void reset_callback() {
//    println("reset");
    ++i;
}

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
    receive_from_nano(Serial1);

    if (i != last) {
        printf("[result]\t%d\n", i);
        last = i;
    }

    delay(5);
}
