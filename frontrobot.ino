#include <Arduino.h>


template<typename T>
class controller_t {
public:
    virtual double operator()(T data) = 0;
};

class pid_t : controller_t<double> {
private:
    double sum{}, last{};
public:
    double kp{}, ki{}, kd{}, integrate_area{}, dead_area{};

    double operator()(double data) override {
        auto value = abs(data);
        if (value < dead_area) {
            sum = .0;
            return .0;
        }
        if (value > integrate_area)
            sum = .0;
        else sum += data;
        auto result = data + kd * (data - last) + ki * sum;
        last = data;
        return kp * result;
    }
};


template<uint8_t pin1, uint8_t pin2>
struct motor_core_t {

    motor_core_t() = delete;

    ~motor_core_t() = delete;

    void state_stop() {
        digitalWrite(pin1, LOW);
        digitalWrite(pin2, LOW);
    }

    void state_break() {
        digitalWrite(pin1, HIGH);
        digitalWrite(pin2, HIGH);
    }

    void pwm(int value) {
        if (value > 0) {
            analogWrite(pin1, value);
            analogWrite(pin2, 0);
        } else {
            analogWrite(pin1, 0);
            analogWrite(pin2, -value);
        }
    }
};

using motor_state_t = uint8_t;

#define STATE_STOP 0x0
#define STATE_BREAK 0x1
#define STATE_SPEED 0x2

template<uint8_t pin1, uint8_t pin2>
class motor_t {
private:
    motor_core_t<pin1, pin2> core;
    bool close_loop = false;
    short ticks = 0, last_ticks = 0;
    float current_speed = .0, target_speed = .0;
public:
    pid_t pid;
    float cpr = 1.0;

    void set_power(int power) {
        core.pwm(power / 255);
    }

    void set_state(uint8_t state) {
        close_loop = false;
        switch (state) {
            case STATE_STOP:
                core.state_stop();
                break;
            case STATE_BREAK:
                core.state_break();
                break;
            case STATE_SPEED:
                close_loop = true;
                break;
            default:
                break;
        }
    }

    void set_speed(float speed) {
        target_speed = speed;
    }

    void run() {
        current_speed = (ticks - last_ticks) / (cpr * 180.0) * PI;
        if (!close_loop)return;
        set_power(pid(target_speed - current_speed));
    }
};


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

using read_result_t=uint8_t;

template<size_t size>
size_t try_read(HardwareSerial serial, uint8_t *buffer, size_t ptr) {
    // 尝试读对应长度
    ptr += serial.readBytes(buffer + 1, size - ptr);
    // 没读完下次再说
    if (ptr < size)
        return ptr;
    uint8_t check = 0;
    // 去掉最后一位（校验位）异或
    for (auto i = ptr - 2; i >= 0; --i)
        check ^= buffer[i];
    // 与最后一位对比
    if (check != buffer[ptr - 1])
        return 0;
    return ptr;
}

void decode_from_nano(HardwareSerial serial) {
    static uint8_t buffer[64];
    static size_t ptr = 0;

    // 是否为新包
    if (ptr == 0)
        while (true) {
            auto byte = serial.read();
            // 前进到头为止
            if (byte == 0xff)
                break;
            else if (byte == -1)
                return;
        }

    // 类型
    buffer[0] = serial.read();
    switch (buffer[0]) {
        case TYPE_SPEED: {
            auto result = try_read<SPEED_SIZE>(serial, buffer, ptr);
            if (result != 0)
                return;
            ptr = 0;
            float speeds[MOTOR_SIZE];
            // 解负载
            memcpy(buffer + 1, speeds, MOTOR_SIZE * sizeof(float));
            speed_callback(speeds);
            break;
        }
        case TYPE_STATE: {
            auto result = try_read<STATE_SIZE>(serial, buffer, ptr);
            if (result != 0)
                return;
            motor_state_t states[MOTOR_SIZE];
            // 解负载
            memcpy(buffer + 1, states, MOTOR_SIZE * sizeof(uint8_t));
            state_callback(states);
            break;
        }
        case TYPE_RESET: {
            auto result = try_read<RESET_SIZE>(serial, buffer, ptr);
            if (result != 0)
                return;
            reset_callback();
            break;
        }
    }
}

void setup() {
    Serial.begin(9600);
}

void loop() {
    delay(2000);
}
