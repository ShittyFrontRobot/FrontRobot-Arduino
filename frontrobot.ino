#include <Arduino.h>


template<typename T>
class Controller {
public:
    virtual double operator()(T data) = 0;
};

class PID : Controller<double> {
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


template<uint8_t a, uint8_t b>
struct motor_core_t {

    motor_core_t() = delete;

    ~motor_core_t() = delete;

    void state_stop() {
        digitalWrite(a, LOW);
        digitalWrite(b, LOW);
    }

    void state_break() {
        digitalWrite(a, HIGH);
        digitalWrite(b, HIGH);
    }

    void pwm(int value) {
        if (value > 0) {
            analogWrite(a, value);
            analogWrite(b, 0);
        } else {
            analogWrite(a, 0);
            analogWrite(b, -value);
        }
    }
};

//
//enum MotorState {
//    Stop, Break, Speed
//};
//MotorState from_raw(uint8_t data) {
//    MotorState state = Stop;
//    switch (data) {
//        case 0x0:
//            state = Stop;
//            break;
//        case 0x1:
//            state = Break;
//            break;
//        case 0x2:
//            state = Speed;
//            break;
//        default:
//            break;
//    }
//    return state;
//}

#define TYPE_SPEED 0xA1
#define SPEED_SIZE 26
#define TYPE_STATE 0xA3
#define STATE_SIZE 8

using chassis_lf = motor_core_t<0, 1>;
using chassis_lb = motor_core_t<0, 1>;
using chassis_rf = motor_core_t<0, 1>;
using chassis_rb = motor_core_t<0, 1>;

void speed_callback(const float *);


void decode_from_nano(HardwareSerial serial) {
    static uint8_t buffer[64];
    static size_t ptr = 0;
    if (ptr == 0)
        while (true) {
            auto byte = serial.read();
            if (byte == 0xff)
                break;
            else if (byte == -1)
                return;
        }

    buffer[0] = serial.read();
    switch (buffer[0]) {
        case TYPE_SPEED: {
            ptr += serial.readBytes(buffer + 1, SPEED_SIZE - ptr);
            if (ptr < SPEED_SIZE)
                return;
            auto size = ptr;
            ptr = 0;
            uint8_t sum = 0;
            for (auto i = size - 2; i >= 0; --i)
                sum ^= buffer[i];
            if (sum != buffer[size - 1])
                return;
            float speeds[4];
            memcpy(buffer + 1, speeds, 4 * sizeof(float));
            speed_callback(speeds);
            break;
        }
        case TYPE_STATE:
            break;
    }
}

void setup() {
    Serial.begin(9600);
}

void loop() {
    uint16_t a = (uint16_t) 0xFF | ((uint16_t) 0xFF << 8);
    Serial.println((int16_t) a);
    delay(2000);
}
